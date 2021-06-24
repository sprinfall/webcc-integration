#ifndef WEBCC_CLIENT_H_
#define WEBCC_CLIENT_H_

#include "webcc/client_base.h"
#include "webcc/socket.h"

namespace webcc {

class Client final : public ClientBase {
public:
  explicit Client(boost::asio::io_context& io_context)
      : ClientBase(io_context) {
  }

  ~Client() = default;

protected:
  void CreateSocket() override {
    socket_.reset(new Socket{ io_context_ });
  }

  void Resolve() override {
    AsyncResolve("80");
  }
};

}  // namespace webcc

#endif  // WEBCC_CLIENT_H_
