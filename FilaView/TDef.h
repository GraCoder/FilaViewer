#pragma once

#define FT_DOWNCAST(CLASS) inline F##CLASS *downcast(CLASS *that) noexcept { return static_cast<F##CLASS *>(that); }

#define FT_FORWARD(CLASS, FUN) void CLASS::##FUN() { return static_cast<F##CLASS *>(this)->##FUN(); }