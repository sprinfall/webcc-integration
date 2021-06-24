#ifndef WEBCC_REQUEST_BUILDER_H_
#define WEBCC_REQUEST_BUILDER_H_

#include <string>
#include <vector>

#include "webcc/fs.h"
#include "webcc/request.h"
#include "webcc/url.h"

// -----------------------------------------------------------------------------
// Handy macros for creating a RequestBuilder.

#define WEBCC_RB webcc::RequestBuilder{}

// clang-format off
#define WEBCC_GET(url)          WEBCC_RB.Get(url, false)
#define WEBCC_GET_ENC(url)      WEBCC_RB.Get(url, true)
#define WEBCC_HEAD(url)         WEBCC_RB.Head(url, false)
#define WEBCC_HEAD_ENC(url)     WEBCC_RB.Head(url, true)
#define WEBCC_POST(url)         WEBCC_RB.Post(url, false)
#define WEBCC_POST_ENC(url)     WEBCC_RB.Post(url, true)
#define WEBCC_PUT(url)          WEBCC_RB.Put(url, false)
#define WEBCC_PUT_ENC(url)      WEBCC_RB.Put(url, true)
#define WEBCC_DELETE(url)       WEBCC_RB.Delete(url, false)
#define WEBCC_DELETE_ENC(url)   WEBCC_RB.Delete(url, true)
#define WEBCC_PATCH(url)        WEBCC_RB.Patch(url, false)
#define WEBCC_PATCH_ENC(url)    WEBCC_RB.Patch(url, true)
// clang-format on

// -----------------------------------------------------------------------------

namespace webcc {

class RequestBuilder {
public:
  RequestBuilder() = default;

  RequestBuilder(const RequestBuilder&) = delete;
  RequestBuilder& operator=(const RequestBuilder&) = delete;

  // Build and return the request object.
  RequestPtr operator()();

  RequestBuilder& Method(string_view method) {
    method_ = ToString(method);
    return *this;
  }

  RequestBuilder& Get(string_view url, bool encode = false) {
    return Method(methods::kGet).Url(url, encode);
  }

  RequestBuilder& Head(string_view url, bool encode = false) {
    return Method(methods::kHead).Url(url, encode);
  }

  RequestBuilder& Post(string_view url, bool encode = false) {
    return Method(methods::kPost).Url(url, encode);
  }

  RequestBuilder& Put(string_view url, bool encode = false) {
    return Method(methods::kPut).Url(url, encode);
  }

  RequestBuilder& Delete(string_view url, bool encode = false) {
    return Method(methods::kDelete).Url(url, encode);
  }

  RequestBuilder& Patch(string_view url, bool encode = false) {
    return Method(methods::kPatch).Url(url, encode);
  }

  RequestBuilder& Url(string_view url, bool encode = false) {
    url_ = webcc::Url{ url, encode };
    return *this;
  }

  RequestBuilder& Port(string_view port) {
    url_.set_port(port);
    return *this;
  }

  RequestBuilder& Port(std::uint16_t port) {
    url_.set_port(std::to_string(port));
    return *this;
  }

  // Append a piece to the path.
  RequestBuilder& Path(string_view path, bool encode = false) {
    url_.AppendPath(path, encode);
    return *this;
  }

  // Append a parameter to the query.
  RequestBuilder& Query(string_view key, string_view value,
                        bool encode = false) {
    url_.AppendQuery(key, value, encode);
    return *this;
  }

  RequestBuilder& MediaType(string_view media_type) {
    media_type_ = ToString(media_type);
    return *this;
  }

  RequestBuilder& Charset(string_view charset) {
    charset_ = ToString(charset);
    return *this;
  }

  // Set Media Type to "application/json".
  RequestBuilder& Json() {
    media_type_ = media_types::kApplicationJson;
    return *this;
  }

  // Set Charset to "utf-8".
  RequestBuilder& Utf8() {
    charset_ = charsets::kUtf8;
    return *this;
  }

  // Set (comma separated) content types to accept.
  // E.g., "application/json", "text/html, application/xhtml+xml".
  RequestBuilder& Accept(string_view content_types) {
    return Header(headers::kAccept, content_types);
  }

#if WEBCC_ENABLE_GZIP

  // Accept Gzip compressed response data or not.
  RequestBuilder& AcceptGzip(bool gzip = true);

#endif  // WEBCC_ENABLE_GZIP

  RequestBuilder& Body(const std::string& data) {
    body_.reset(new StringBody{ data, false });
    return *this;
  }

  RequestBuilder& Body(std::string&& data) {
    body_.reset(new StringBody{ std::move(data), false });
    return *this;
  }

  // Use the file content as body.
  // NOTE: Error::kFileError might be thrown.
  RequestBuilder& File(const fs::path& path, bool infer_media_type = true,
                       std::size_t chunk_size = 1024);

  // Add a form part.
  RequestBuilder& Form(FormPartPtr part) {
    form_parts_.push_back(part);
    return *this;
  }

  // Add a form part of file.
  RequestBuilder& FormFile(string_view name, const fs::path& path,
                           string_view media_type = "");

  // Add a form part of string data.
  RequestBuilder& FormData(string_view name, std::string&& data,
                           string_view media_type = "");

  RequestBuilder& Header(string_view key, string_view value);

  RequestBuilder& KeepAlive(bool keep_alive = true) {
    keep_alive_ = keep_alive;
    return *this;
  }

  RequestBuilder& Auth(string_view type, string_view credentials);

  RequestBuilder& AuthBasic(string_view login, string_view password);

  RequestBuilder& AuthToken(string_view token);

  // Add the `Date` header to the request.
  RequestBuilder& Date();

#if WEBCC_ENABLE_GZIP

  // Compress the body data (only for string body).
  // NOTE:
  // Most servers don't support compressed requests.
  // Even the requests module from Python doesn't have a built-in support.
  // See: https://github.com/kennethreitz/requests/issues/1753
  RequestBuilder& Gzip(bool gzip = true) {
    gzip_ = gzip;
    return *this;
  }

#endif  // WEBCC_ENABLE_GZIP

private:
  std::string method_;

  // Namespace is added to avoid the conflict with `Url()` method.
  webcc::Url url_;

  // Request body.
  BodyPtr body_;

  // The media (or MIME) type of `Content-Type` header.
  // E.g., "application/json".
  std::string media_type_;

  // The charset of `Content-Type` header.
  // E.g., "utf-8".
  std::string charset_;

  // Files to upload for a POST request.
  std::vector<FormPartPtr> form_parts_;

  // Additional headers with the following sequence:
  //   { key1, value1, key2, value2, ... }
  Strings headers_;

  // Persistent connection.
  bool keep_alive_ = true;

#if WEBCC_ENABLE_GZIP
  bool gzip_ = false;
#endif  // WEBCC_ENABLE_GZIP
};

}  // namespace webcc

#endif  // WEBCC_REQUEST_BUILDER_H_
