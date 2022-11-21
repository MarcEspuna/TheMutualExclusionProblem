#pragma once
#define WIN32_LEAN_AND_MEAN
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW
#endif

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <thread>
#include <future>
#include <condition_variable>
#include <atomic>
#include <array>
#include <mutex>
#include <stdio.h>
#include <initializer_list>
#include <WinSock2.h>
#include <windows.h>
#include <unordered_map>
#include <queue>