#pragma once

#include <exception>

template <typename T>
class PtrInternalInterface;

template <typename T, typename D>
class PtrInternal;

template <typename T>
class Ptr;

struct ControlBlockBase;

template <typename T>
struct ControlBlockWithObject;

template <class Derived>
struct ControlBlockWithPointer;

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
