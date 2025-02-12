#pragma once

#include <exception>

class emptyTreeException : public std::exception {
  const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
    return "emptyTree";
  }
};

class queueLoopException : public std::exception {
  const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
    return "queueLoopException";
  }
};

class queueLoopReplacementDetectedException : public std::exception {
  const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
    return "queueLoopReplacementDetectedException";
  }
};

class loadLoopException : public std::exception {
  const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
    return "loadLoopException";
  }
};

class placeLoopException : public std::exception {
  const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
    return "placeLoopException";
  }
};

class failPeerConflictException : public std::exception {
  const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
    return "failPeerConflict";
  }
};

class npmErrorException : public std::exception {
  const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
    return "npmError";
  }
};
