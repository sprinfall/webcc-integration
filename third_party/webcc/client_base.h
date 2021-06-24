#ifndef WEBCC_CLIENT_BASE_H_
#define WEBCC_CLIENT_BASE_H_

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "boost/asio/io_context.hpp"
#include "boost/asio/ip/tcp.hpp"
#include "boost/asio/steady_timer.hpp"

#include "webcc/globals.h"
#include "webcc/request.h"
#include "webcc/response.h"
#include "webcc/response_parser.h"
#include "webcc/socket_base.h"

namespace webcc {

class ClientBase {
public:
  explicit ClientBase(boost::asio::io_context& io_context);

  ClientBase(const ClientBase&) = delete;
  ClientBase& operator=(const ClientBase&) = delete;

  ~ClientBase() = default;

  void set_buffer_size(std::size_t buffer_size) {
    if (buffer_size > 0) {
      buffer_size_ = buffer_size;
    }
  }

  void set_connect_timeout(int timeout) {
    if (timeout > 0) {
      connect_timeout_ = timeout;
    }
  }

  void set_read_timeout(int timeout)  {
    if (timeout > 0) {
      read_timeout_ = timeout;
    }
  }

  // Set progress callback to be informed about the read progress.
  // NOTE: Don't use move semantics because in practice, there is no difference
  //       between copying and moving an object of a closure type.
  // TODO: Support write progress
  void set_progress_callback(ProgressCallback callback) {
    progress_callback_ = callback;
  }

  // Connect, send request, wait until response is received.
  Error Request(RequestPtr request, bool stream = false);

  // Close the connection.
  // The async operation on the socket will be canceled.
  void Close();

  bool connected() const {
    return connected_;
  }

  ResponsePtr response() const {
    return response_;
  }

  // Reset response object.
  // Used to make sure the response object will released even the client object
  // itself will be cached for keep-alive purpose.
  void Reset() {
    response_.reset();
    response_parser_.Init(nullptr, false);
  }

protected:
  // Create Socket or SslSocket.
  virtual void CreateSocket() = 0;

  // Resolve host.
  virtual void Resolve() = 0;

  void CloseSocket();

  void AsyncResolve(string_view default_port);

  void OnResolve(boost::system::error_code ec,
                 boost::asio::ip::tcp::resolver::results_type endpoints);

  void OnConnect(boost::system::error_code ec, boost::asio::ip::tcp::endpoint);

  void AsyncWrite();
  void OnWrite(boost::system::error_code ec, std::size_t length);

  void AsyncWriteBody();
  void OnWriteBody(boost::system::error_code ec, std::size_t length);

  void HandleWriteError(boost::system::error_code ec);

  void AsyncRead();
  void OnRead(boost::system::error_code ec, std::size_t length);

  void AsyncWaitDeadlineTimer(int seconds);
  void OnDeadlineTimer(boost::system::error_code ec);
  void StopDeadlineTimer();

  void FinishRequest();

protected:
  boost::asio::io_context& io_context_;

  std::unique_ptr<SocketBase> socket_;

  boost::asio::ip::tcp::resolver resolver_;

  bool request_finished_ = true;
  std::condition_variable request_cv_;
  std::mutex request_mutex_;

  RequestPtr request_;

  ResponsePtr response_;
  ResponseParser response_parser_;

  // The length already read.
  std::size_t length_read_ = 0;

  // The buffer for reading response.
  std::vector<char> buffer_;

  // The size of the buffer for reading response.
  // 0 means default value will be used.
  std::size_t buffer_size_ = kBufferSize;

  // Timeout (seconds) for connecting to server.
  // Default as 0 to disable our own control (i.e., deadline_timer_).
  int connect_timeout_ = 0;

  // Timeout (seconds) for reading response.
  int read_timeout_ = kMaxReadSeconds;

  // Deadline timer for connecting to server.
  boost::asio::steady_timer deadline_timer_;
  bool deadline_timer_stopped_ = true;

  // Socket connected or not.
  bool connected_ = false;

  // Progress callback (optional).
  ProgressCallback progress_callback_;

  // Current error.
  Error error_;
};

using ClientPtr = std::shared_ptr<ClientBase>;

}  // namespace webcc

#endif  // WEBCC_CLIENT_BASE_H_
