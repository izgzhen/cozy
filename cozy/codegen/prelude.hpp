#include "assert.h"
#include <vector>
#include <memory>

template <typename T>
class Stream {
public:
  virtual bool tryget(int& idx, T& out) = 0;

  std::vector<T> collect() {
    std::vector<T> ret;
    _foreach([&](T e) {
      ret.push_back(e);
      return false;
    });
    return ret;
  }

  template <class F>
  void foreach(const F& _callback) {
    _foreach([&](T e) {
      _callback(e);
      return false;
    });
  }

  T get(int idx) {
    T ret;
    assert(tryget(idx, ret));
    return ret;
  }

  virtual bool _foreach(std::function<bool (T)>) = 0;
};

template <typename T>
class FilterStream final : public Stream<T> {
  std::shared_ptr<Stream<T>> _s;
  std::shared_ptr<std::function<bool (T)>> _cond;
public:
  FilterStream(std::shared_ptr<Stream<T>> s, std::function<bool (T)> cond) :
    _s(s), _cond(std::make_shared<std::function<bool (T)>>(cond)) {}

  bool tryget(int& idx, T& out) {
    return _s->_foreach([&](T e) {
      if ((*_cond)(e)) {
        if (idx == 0) {
          out = e;
          return true;
        }
        idx -= 1;
      }
      return false;
    });
  }

  bool _foreach(std::function<bool (T)> cb) {
    return _s->_foreach([&](T e) {
      if ((*_cond)(e)) {
        if (cb(e)) {
          return true;
        }
      }
      return false;
    });
  }
};

template <typename T>
class SliceStream final : public Stream<T> {
  std::shared_ptr<Stream<T>> _s;
  int _start;
  int _end;
  bool _has_end;

public:
  SliceStream(std::shared_ptr<Stream<T>> s, int start, int end) :
    _s(s), _start(start), _end(end), _has_end(true) {}

  SliceStream(std::shared_ptr<Stream<T>> s, int start) :
    _s(s), _start(start), _has_end(false) {}

  bool tryget(int& idx, T& out) {
    if (_has_end && idx >= _end - _start) {
      return false;
    }
    idx += _start;
    return _s->tryget(idx, out);
  }

  bool _foreach(std::function<bool (T)> cb) {
    int i = 0;
    return _s->_foreach([&](T e) {
      if (i >= _start && ((_has_end && i < _end) || (!_has_end))) {
        if (cb(e)) {
          return true;
        }
      }
      i++;
      return false;
    });
  }
};

template <typename T>
class ConcatStream final : public Stream<T> {
  std::shared_ptr<Stream<T>> _s1;
  std::shared_ptr<Stream<T>> _s2;
public:
  ConcatStream(std::shared_ptr<Stream<T>> s1, std::shared_ptr<Stream<T>> s2) :
    _s1(s1), _s2(s2) { }

  bool tryget(int& idx, T& out) {
    if (_s1->tryget(idx, out)) {
      return true;
    }
    return _s2->tryget(idx, out);
  }

  bool _foreach(std::function<bool (T)> _callback) {
    if (_s1->_foreach(_callback)) {
      return true;
    }
    return _s2->_foreach(_callback);
  }
};


template <typename T>
class VecStream final : public Stream<T> {
  typedef typename std::vector<T> vec_t;
  const vec_t& _l;

public:
  VecStream(const vec_t& l) : _l(l) { }

  bool tryget(int& idx, T& out) {
    if (_l.size() > idx) {
      out = _l[idx];
      return true;
    }
    idx -= _l.size();
    return false;
  }

  bool _foreach(std::function<bool (T)> _callback) {
    for (auto x : _l) {
      if (_callback(x)) {
        return true;
      }
    }
    return false;
  }
};

template <typename T>
class ConcreteVecStream final : public Stream<T> {
  typedef typename std::vector<T> vec_t;
  vec_t _l;
  std::shared_ptr<VecStream<T>> _s;

public:
  ConcreteVecStream(vec_t l) {
    _l = l;
    _s = std::make_shared<VecStream<T>>(_l);
  }
  ConcreteVecStream() {
    _l = std::vector<T>();
    _s = std::make_shared<VecStream<T>>(_l);
  }

  bool tryget(int& idx, T& out) {
    return _s->tryget(idx, out);
  }

  bool _foreach(std::function<bool (T)> _callback) {
    return _s->_foreach(_callback);
  }
};
