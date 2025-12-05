/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ImGuiHelper.h"

#include <unordered_map>
#include <vector>

#include "imgui.h"

#include <filament/Camera.h>
#include <filament/Fence.h>
#include <filament/IndexBuffer.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/Texture.h>
#include <filament/TextureSampler.h>
#include <filament/TransformManager.h>
#include <filament/VertexBuffer.h>

#include <utils/EntityManager.h>

using namespace filament::math;
using namespace filament;
using namespace utils;

using MinFilter = TextureSampler::MinFilter;
using MagFilter = TextureSampler::MagFilter;

namespace filagui {

#include "pcv_mat.h"

ImGuiHelper::ImGuiHelper(Engine *engine, filament::View *view, const Path &fontPath, ImGuiContext *imGuiContext)
  : mEngine(engine)
  , mView(view)
  , mScene(engine->createScene())
  , mImGuiContext(imGuiContext ? imGuiContext : ImGui::CreateContext())
{
  ImGuiIO &io = ImGui::GetIO();
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
  mSettingsPath.setPath(Path::getUserSettingsDirectory() + Path(std::string(".") + Path::getCurrentExecutable().getNameWithoutExtension()) +
                        Path("imgui_settings.ini"));
  mSettingsPath.getParent().mkdirRecursive();
  io.IniFilename = mSettingsPath.c_str();

  // Create a simple alpha-blended 2D blitting material.
  mMaterial = Material::Builder().package(PCV_MAT_UI_DATA, PCV_MAT_UI_SIZE).build(*engine);

  // If the given font path is invalid, ImGui will silently fall back to proggy, which is a
  // tiny "pixel art" texture that is compiled into the library.
  if (fontPath.isFile()) {
    io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 16.0f);
  }
  createAtlasTexture(engine);

  // For proggy, switch to NEAREST for pixel-perfect text.
  if (!fontPath.isFile() && !imGuiContext) {
    mSampler = TextureSampler(MinFilter::NEAREST, MagFilter::NEAREST);
    mMaterial->setDefaultParameter("albedo", mTexture, mSampler);
  }

  utils::EntityManager &em = utils::EntityManager::get();
  mCameraEntity = em.create();
  mCamera = mEngine->createCamera(mCameraEntity);

  view->setCamera(mCamera);

  view->setPostProcessingEnabled(false);
  view->setBlendMode(View::BlendMode::TRANSLUCENT);
  view->setShadowingEnabled(false);

  // Attach a scene for our one and only Renderable.
  view->setScene(mScene);

  _entity = em.create();
  mScene->addEntity(_entity);

  ImGui::StyleColorsDark();
}

ImGuiHelper::~ImGuiHelper()
{
  mEngine->destroy(mScene);
  mEngine->destroy(_entity);
  mEngine->destroyCameraComponent(mCameraEntity);

  mEngine->destroy(mTexture);
  for (auto &vb : mVertexBuffers) {
    mEngine->destroy(vb);
  }
  for (auto &ib : mIndexBuffers) {
    mEngine->destroy(ib);
  }

  EntityManager &em = utils::EntityManager::get();
  em.destroy(_entity);
  em.destroy(mCameraEntity);

  for (auto &mi : mMaterialInstances) {
    mEngine->destroy(mi);
  }
  mEngine->destroy(mMaterial);

  ImGui::DestroyContext(mImGuiContext);
  mImGuiContext = nullptr;
}

void ImGuiHelper::setDisplaySize(int width, int height, float scaleX, float scaleY, bool flipVertical)
{
  ImGuiIO &io = ImGui::GetIO();
  io.DisplaySize = ImVec2(width, height);
  io.DisplayFramebufferScale.x = scaleX;
  io.DisplayFramebufferScale.y = scaleY;
  mFlipVertical = flipVertical;
  mCamera->lookAt({0, 0, 2}, {0, 0, 0});
  if (flipVertical) {
    mCamera->setProjection(Camera::Projection::ORTHO, 0.0, double(width), 0.0, double(height), 1.0, 3.0);
  } else {
    mCamera->setProjection(Camera::Projection::ORTHO, 0.0, double(width), double(height), 0.0, 1.0, 3.0);
  }
}

void ImGuiHelper::render(float timeStepInSeconds, Callback imguiCommands)
{
  ImGui::SetCurrentContext(mImGuiContext);
  ImGuiIO &io = ImGui::GetIO();
  //io.DeltaTime = timeStepInSeconds;
  // First, let ImGui process events and increment its internal frame count.
  // This call will also update the io.WantCaptureMouse, io.WantCaptureKeyboard flag
  // that tells us whether to dispatch inputs (or not) to the app.
  ImGui::NewFrame();
  // Allow the client app to create widgets.
  imguiCommands(mEngine, mView);
  // Let ImGui build up its draw data.
  ImGui::Render();
  // Finally, translate the draw data into Filament objects.
  processImGuiCommands(ImGui::GetDrawData(), ImGui::GetIO());
}

void ImGuiHelper::processImGuiCommands(ImDrawData *commands, const ImGuiIO &io)
{
  ImGui::SetCurrentContext(mImGuiContext);

  mHasSynced = false;
  auto &rcm = mEngine->getRenderableManager();

  // Avoid rendering when minimized and scale coordinates for retina displays.
  int fbwidth = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
  int fbheight = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
  if (fbwidth == 0 || fbheight == 0)
    return;
  commands->ScaleClipRects(io.DisplayFramebufferScale);

  // Ensure that we have enough vertex buffers and index buffers.
  createBuffers(commands->CmdListsCount);

  // Count how many primitives we'll need, then create a Renderable builder.
  size_t nPrims = 0;
  std::unordered_map<uint64_t, filament::MaterialInstance *> scissorRects;
  for (int cmdListIndex = 0; cmdListIndex < commands->CmdListsCount; cmdListIndex++) {
    const ImDrawList *cmds = commands->CmdLists[cmdListIndex];
    nPrims += cmds->CmdBuffer.size();
  }
  auto rbuilder = RenderableManager::Builder(nPrims);
  rbuilder.boundingBox({{0, 0, 0}, {10000, 10000, 10000}}).culling(false);

  // Ensure that we have a material instance for each primitive.
  size_t previousSize = mMaterialInstances.size();
  if (nPrims > mMaterialInstances.size()) {
    mMaterialInstances.resize(nPrims);
    for (size_t i = previousSize; i < mMaterialInstances.size(); i++) {
      mMaterialInstances[i] = mMaterial->createInstance();
    }
  }

  // Recreate the Renderable component and point it to the vertex buffers.
  rcm.destroy(_entity);
  int bufferIndex = 0;
  int primIndex = 0;
  for (int cmdListIndex = 0; cmdListIndex < commands->CmdListsCount; cmdListIndex++) {
    const ImDrawList *cmds = commands->CmdLists[cmdListIndex];
    populateVertexData(bufferIndex, cmds->VtxBuffer.Size * sizeof(ImDrawVert), 
      cmds->VtxBuffer.Data, cmds->IdxBuffer.Size * sizeof(ImDrawIdx), cmds->IdxBuffer.Data);
    for (const auto &pcmd : cmds->CmdBuffer) {
      if (pcmd.UserCallback) {
        pcmd.UserCallback(cmds, &pcmd);
        continue;
      }

      MaterialInstance *materialInstance = mMaterialInstances[primIndex];
      materialInstance->setScissor(pcmd.ClipRect.x, mFlipVertical ? pcmd.ClipRect.y : (fbheight - pcmd.ClipRect.w),
                                   (uint16_t)(pcmd.ClipRect.z - pcmd.ClipRect.x), (uint16_t)(pcmd.ClipRect.w - pcmd.ClipRect.y));
      //if (pcmd.GetTexID()) {
      //  TextureSampler sampler(MinFilter::LINEAR, MagFilter::LINEAR);
      //  materialInstance->setParameter("albedo", (Texture const *)pcmd.GetTexID(), sampler);
      //} else {
      //  materialInstance->setParameter("albedo", mTexture, mSampler);
      //}
      rbuilder
        .geometry(primIndex, RenderableManager::PrimitiveType::TRIANGLES, mVertexBuffers[bufferIndex], mIndexBuffers[bufferIndex], pcmd.IdxOffset, pcmd.ElemCount)
        .blendOrder(primIndex, primIndex)
        .material(primIndex, materialInstance);
      primIndex++;
    }
    bufferIndex++;
  }
  if (commands->CmdListsCount > 0) {
    rbuilder.build(*mEngine, _entity);
  }
}

void ImGuiHelper::createAtlasTexture(Engine *engine)
{
  engine->destroy(mTexture);
  ImGuiIO &io = ImGui::GetIO();
  // Create the grayscale texture that ImGui uses for its glyph atlas.
  static unsigned char *pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  size_t size = (size_t)(width * height * 4);
  Texture::PixelBufferDescriptor pb(pixels, size, Texture::Format::RGBA, Texture::Type::UBYTE);
  mTexture = Texture::Builder()
               .width((uint32_t)width)
               .height((uint32_t)height)
               .levels((uint8_t)1)
               .format(Texture::InternalFormat::RGBA8)
               .sampler(Texture::Sampler::SAMPLER_2D)
               .build(*engine);
  mTexture->setImage(*engine, 0, std::move(pb));

  mSampler = TextureSampler(MinFilter::LINEAR, MagFilter::LINEAR);
  mMaterial->setDefaultParameter("albedo", mTexture, mSampler);
}

namespace {
static constexpr int mk[4] = {0, 0, 1, 2};

ImGuiKey convert_key(int scanCode)
{
  switch (scanCode) {
  case SDL_SCANCODE_TAB:
    return ImGuiKey_Tab;
  case SDL_SCANCODE_LEFT:
    return ImGuiKey_LeftArrow;
  case SDL_SCANCODE_RIGHT:
    return ImGuiKey_RightArrow;
  case SDL_SCANCODE_UP:
    return ImGuiKey_UpArrow;
  case SDL_SCANCODE_DOWN:
    return ImGuiKey_DownArrow;
  case SDL_SCANCODE_PAGEUP:
    return ImGuiKey_PageUp;
  case SDL_SCANCODE_PAGEDOWN:
    return ImGuiKey_PageDown;
  case SDL_SCANCODE_HOME:
    return ImGuiKey_Home;
  case SDL_SCANCODE_END:
    return ImGuiKey_End;
  case SDL_SCANCODE_INSERT:
    return ImGuiKey_Insert;
  case SDL_SCANCODE_DELETE:
    return ImGuiKey_Delete;
  case SDL_SCANCODE_BACKSPACE:
    return ImGuiKey_Backspace;
  case SDL_SCANCODE_SPACE:
    return ImGuiKey_Space;
  case SDL_SCANCODE_RETURN:
    return ImGuiKey_Enter;
  case SDL_SCANCODE_ESCAPE:
    return ImGuiKey_Escape;

  // 字母
  case SDL_SCANCODE_A:
    return ImGuiKey_A;
  case SDL_SCANCODE_C:
    return ImGuiKey_C;
  case SDL_SCANCODE_V:
    return ImGuiKey_V;
  case SDL_SCANCODE_X:
    return ImGuiKey_X;
  case SDL_SCANCODE_Y:
    return ImGuiKey_Y;
  case SDL_SCANCODE_Z:
    return ImGuiKey_Z;

  // 数字（上排）
  case SDL_SCANCODE_0:
    return ImGuiKey_0;
  case SDL_SCANCODE_1:
    return ImGuiKey_1;
  case SDL_SCANCODE_2:
    return ImGuiKey_2;
  case SDL_SCANCODE_3:
    return ImGuiKey_3;
  case SDL_SCANCODE_4:
    return ImGuiKey_4;
  case SDL_SCANCODE_5:
    return ImGuiKey_5;
  case SDL_SCANCODE_6:
    return ImGuiKey_6;
  case SDL_SCANCODE_7:
    return ImGuiKey_7;
  case SDL_SCANCODE_8:
    return ImGuiKey_8;
  case SDL_SCANCODE_9:
    return ImGuiKey_9;

  // 功能键
  case SDL_SCANCODE_F1:
    return ImGuiKey_F1;
  case SDL_SCANCODE_F2:
    return ImGuiKey_F2;
  case SDL_SCANCODE_F3:
    return ImGuiKey_F3;
  case SDL_SCANCODE_F4:
    return ImGuiKey_F4;
  case SDL_SCANCODE_F5:
    return ImGuiKey_F5;
  case SDL_SCANCODE_F6:
    return ImGuiKey_F6;
  case SDL_SCANCODE_F7:
    return ImGuiKey_F7;
  case SDL_SCANCODE_F8:
    return ImGuiKey_F8;
  case SDL_SCANCODE_F9:
    return ImGuiKey_F9;
  case SDL_SCANCODE_F10:
    return ImGuiKey_F10;
  case SDL_SCANCODE_F11:
    return ImGuiKey_F11;
  case SDL_SCANCODE_F12:
    return ImGuiKey_F12;
  default:
    return ImGuiKey_None;
  }
}

void addMod(ImGuiIO &io, int mod, bool press) 
{
  // Control (Ctrl)
  if (mod & KMOD_LCTRL)
    io.AddKeyEvent(ImGuiKey_LeftCtrl, press);
  if (mod & KMOD_RCTRL)
    io.AddKeyEvent(ImGuiKey_RightCtrl, press);
  if ((mod & KMOD_CTRL) && !(mod & (KMOD_LCTRL | KMOD_RCTRL))) {
    // generic CTRL set but no side-specific bits -> set both to be safe
    io.AddKeyEvent(ImGuiKey_LeftCtrl, press);
    io.AddKeyEvent(ImGuiKey_RightCtrl, press);
  }

  // Shift
  if (mod & KMOD_LSHIFT)
    io.AddKeyEvent(ImGuiKey_LeftShift, press);
  if (mod & KMOD_RSHIFT)
    io.AddKeyEvent(ImGuiKey_RightShift, press);
  if ((mod & KMOD_SHIFT) && !(mod & (KMOD_LSHIFT | KMOD_RSHIFT))) {
    io.AddKeyEvent(ImGuiKey_LeftShift, press);
    io.AddKeyEvent(ImGuiKey_RightShift, press);
  }

  // Alt (including AltGr)
  if (mod & KMOD_LALT)
    io.AddKeyEvent(ImGuiKey_LeftAlt, press);
  if (mod & KMOD_RALT)
    io.AddKeyEvent(ImGuiKey_RightAlt, press);
  if ((mod & KMOD_ALT) && !(mod & (KMOD_LALT | KMOD_RALT))) {
    io.AddKeyEvent(ImGuiKey_LeftAlt, press);
    io.AddKeyEvent(ImGuiKey_RightAlt, press);
  }

  // GUI / Super (Windows / Command)
  if (mod & KMOD_LGUI)
    io.AddKeyEvent(ImGuiKey_LeftSuper, press);
  if (mod & KMOD_RGUI)
    io.AddKeyEvent(ImGuiKey_RightSuper, press);
  if ((mod & KMOD_GUI) && !(mod & (KMOD_LGUI | KMOD_RGUI))) {
    io.AddKeyEvent(ImGuiKey_LeftSuper, press);
    io.AddKeyEvent(ImGuiKey_RightSuper, press);
  }

  // Lock keys
  if (mod & KMOD_CAPS)
    io.AddKeyEvent(ImGuiKey_CapsLock, press);
  if (mod & KMOD_NUM)
    io.AddKeyEvent(ImGuiKey_NumLock, press);

  // KMOD_MODE is often AltGr on some layouts; treat it as RightAlt for ImGui
  if (mod & KMOD_MODE)
    io.AddKeyEvent(ImGuiKey_RightAlt, press);
}

}

bool ImGuiHelper::keyDn(const SDL_KeyboardEvent &keyEvent)
{
  int key = convert_key(keyEvent.keysym.scancode);
  auto &io = ImGui::GetIO();
  io.AddKeyEvent(static_cast<ImGuiKey>(key), true);
  if (keyEvent.keysym.mod > 0)
    addMod(io, keyEvent.keysym.mod, true);
  if (io.WantCaptureKeyboard)
    return true;
  return false;
}

bool ImGuiHelper::keyUp(const SDL_KeyboardEvent &keyEvent)
{
  ImGuiKey key = convert_key(keyEvent.keysym.scancode);

  auto &io = ImGui::GetIO();
  io.AddKeyEvent(static_cast<ImGuiKey>(key), false);
  if (keyEvent.keysym.mod > 0)
    addMod(io, keyEvent.keysym.mod, false);
  if (io.WantCaptureKeyboard)
    return true;
  return false;
}

bool ImGuiHelper::inputText(const char *text)
{
  auto &io = ImGui::GetIO();
  io.AddInputCharactersUTF8(text);
  return io.WantCaptureKeyboard;
}

bool ImGuiHelper::mouseButtonDn(const SDL_MouseButtonEvent &mEvent)
{
  auto &io = ImGui::GetIO();
  io.AddMousePosEvent(mEvent.x, mEvent.y);
  io.AddMouseButtonEvent(mk[mEvent.button], true);
  if (io.WantCaptureMouse)
    return true;

  return false;
}

bool ImGuiHelper::mouseButtonUp(const SDL_MouseButtonEvent &mEvent)
{
  auto &io = ImGui::GetIO();
  io.AddMousePosEvent(mEvent.x, mEvent.y);
  io.AddMouseButtonEvent(mk[mEvent.button], false);
  if (io.WantCaptureMouse)
    return true;

  return false;
}

bool ImGuiHelper::mouseMove(const SDL_MouseMotionEvent &mEvent)
{
  auto &io = ImGui::GetIO();
  io.AddMousePosEvent(mEvent.x, mEvent.y);
  if (io.WantCaptureMouse)
    return true;

  return false;
}

bool ImGuiHelper::mouseWheel(const SDL_MouseWheelEvent &wEvent)
{
  auto &io = ImGui::GetIO();
  io.AddMouseWheelEvent(wEvent.mouseX, wEvent.mouseY);
  if (io.WantCaptureMouse)
    return true;

  return false;
}

void ImGuiHelper::createBuffers(int numRequiredBuffers)
{
  if (numRequiredBuffers > mVertexBuffers.size()) {
    size_t previousSize = mVertexBuffers.size();
    mVertexBuffers.resize(numRequiredBuffers, nullptr);
    for (size_t i = previousSize; i < mVertexBuffers.size(); i++) {
      // Pick a reasonable starting capacity; it will grow if needed.
      createVertexBuffer(i, 1000);
    }
  }
  if (numRequiredBuffers > mIndexBuffers.size()) {
    size_t previousSize = mIndexBuffers.size();
    mIndexBuffers.resize(numRequiredBuffers, nullptr);
    for (size_t i = previousSize; i < mIndexBuffers.size(); i++) {
      // Pick a reasonable starting capacity; it will grow if needed.
      createIndexBuffer(i, 5000);
    }
  }
}

void ImGuiHelper::populateVertexData(size_t bufferIndex, size_t vbSizeInBytes, void *vbImguiData, size_t ibSizeInBytes, void *ibImguiData)
{
  // Create a new vertex buffer if the size isn't large enough, then copy the ImGui data into
  // a staging area since Filament's render thread might consume the data at any time.
  size_t requiredVertCount = vbSizeInBytes / sizeof(ImDrawVert);
  size_t capacityVertCount = mVertexBuffers[bufferIndex]->getVertexCount();
  if (requiredVertCount > capacityVertCount) {
    createVertexBuffer(bufferIndex, requiredVertCount);
  }
  size_t nVbBytes = requiredVertCount * sizeof(ImDrawVert);
  void *vbFilamentData = malloc(nVbBytes);
  memcpy(vbFilamentData, vbImguiData, nVbBytes);
  mVertexBuffers[bufferIndex]->setBufferAt(
    *mEngine, 0, VertexBuffer::BufferDescriptor(vbFilamentData, nVbBytes, [](void *buffer, size_t size, void *user) { free(buffer); }, /* user = */ nullptr));

  // Create a new index buffer if the size isn't large enough, then copy the ImGui data into
  // a staging area since Filament's render thread might consume the data at any time.
  size_t requiredIndexCount = ibSizeInBytes / 2;
  size_t capacityIndexCount = mIndexBuffers[bufferIndex]->getIndexCount();
  if (requiredIndexCount > capacityIndexCount) {
    createIndexBuffer(bufferIndex, requiredIndexCount);
  }
  size_t nIbBytes = requiredIndexCount * 2;
  void *ibFilamentData = malloc(nIbBytes);
  memcpy(ibFilamentData, ibImguiData, nIbBytes);
  mIndexBuffers[bufferIndex]->setBuffer(
    *mEngine, IndexBuffer::BufferDescriptor(ibFilamentData, nIbBytes, [](void *buffer, size_t size, void *user) { free(buffer); }, /* user = */ nullptr));
}

void ImGuiHelper::createVertexBuffer(size_t bufferIndex, size_t capacity)
{
  syncThreads();
  mEngine->destroy(mVertexBuffers[bufferIndex]);
  mVertexBuffers[bufferIndex] =
    VertexBuffer::Builder()
      .vertexCount(capacity)
      .bufferCount(1)
      .attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, sizeof(ImDrawVert))
      .attribute(VertexAttribute::UV0, 0, VertexBuffer::AttributeType::FLOAT2, sizeof(filament::math::float2), sizeof(ImDrawVert))
      .attribute(VertexAttribute::COLOR, 0, VertexBuffer::AttributeType::UBYTE4, 2 * sizeof(filament::math::float2), sizeof(ImDrawVert))
      .normalized(VertexAttribute::COLOR)
      .build(*mEngine);
}

void ImGuiHelper::createIndexBuffer(size_t bufferIndex, size_t capacity)
{
  syncThreads();
  mEngine->destroy(mIndexBuffers[bufferIndex]);
  mIndexBuffers[bufferIndex] = IndexBuffer::Builder().indexCount(capacity).bufferType(IndexBuffer::IndexType::USHORT).build(*mEngine);
}

void ImGuiHelper::syncThreads()
{
#if UTILS_HAS_THREADING
  if (!mHasSynced) {
    // This is called only when ImGui needs to grow a vertex buffer, which occurs a few times
    // after launching and rarely (if ever) after that.
    Fence::waitAndDestroy(mEngine->createFence());
    mHasSynced = true;
  }
#endif
}

} // namespace filagui
