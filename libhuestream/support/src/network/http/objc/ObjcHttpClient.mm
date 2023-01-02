/*******************************************************************************
 Copyright (C) 2019 Signify Holding
 All Rights Reserved.
 ********************************************************************************/

#import <Foundation/Foundation.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>

#include "support/date/Date.h"

#include "support/network/NetworkConfiguration.h"
#include "support/network/http/objc/ObjcHttpClient.h"
#include "support/network/http/HttpResponse.h"
#include "support/network/http/util/X509Certificate.h"
#include "support/threading/Thread.h"

using std::atomic;
using std::condition_variable;
using std::mutex;
using std::string;
using std::unique_ptr;

namespace support {
    class ObjcHttpRequest;
};

@interface HueURLSessionTaskDelegate : NSObject<NSURLSessionTaskDelegate>

@property support::ObjcHttpRequest* request;

- (id) initWithHttpRequest:(support::ObjcHttpRequest*) request;

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task willPerformHTTPRedirection:(NSHTTPURLResponse *)response newRequest:(NSURLRequest *)request completionHandler:(void (^)(NSURLRequest *_Nullable))completionHandler;

- (void)URLSession:(NSURLSession *)session didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition, NSURLCredential *))completionHandler;

@end

namespace support {

    class ObjcHttpRequest {

    public:
        ObjcHttpRequest(const HttpRequestParams& data, HttpRequestCallback callback);

        ~ObjcHttpRequest();

        int do_request_internal(const string&       method,
                const string&       body,
                File*               file,
                HttpRequestCallback callback);

        void set_ssl_error_cancellation(bool ssl_error_cancellation) {
            _ssl_error_cancellation = ssl_error_cancellation;
        }

        const std::string& common_name() const {
            return _common_name;
        }

        const std::vector<std::string>& trusted_certs() const {
            return _trusted_certs;
        }

        bool verify_ssl() const {
            return _verify_ssl;
        }

        void add_certificates_to_chain(const std::vector<std::string> &certificates) {
            _certificate_chain.insert(_certificate_chain.end(), certificates.begin(), certificates.end());
        }

    private:
        /**
         Get dictionary with the proxy settings
         @return the dictionary with the proxy settings
         */
        NSDictionary* get_proxy_address_dictionary(string proxy_address, int proxy_port);

        /**
         Get url session with configuration
         @return the url session
         */
        NSURLSession* get_url_session(string proxy_address, int proxy_port);

        /**
         Get raw header string based on the values
         in the dictionary
         @return The string representation
         */
        string get_header_string(NSDictionary* headers);

        static const unsigned int HTTP_RESPONSE_CODE_GATEWAY_TIMEOUT = 504;

        int                      _receive_timeout;
        int                      _request_timeout;
        log::LogComponentType    _log_component;
        std::string              _proxy_address;
        int                      _proxy_port;
        std::string              _common_name;
        std::vector<std::string> _trusted_certs;
        bool                     _verify_ssl;
        std::vector<std::string> _certificate_chain;

        std::mutex               _executing_mutex;
        atomic<HttpRequestState> _state;
        std::unique_ptr<Thread>  _executing_thread;

        NSURLSessionTask*    _session_task;
        NSMutableURLRequest* _request;

        bool _ssl_error_cancellation = false;
        HueURLSessionTaskDelegate* _session_delegate;

        void cancel();

        void start_execution_thread(HttpRequestCallback callback);

        /**
         * Create a request error object from the HTTP status code (Note: can be a success "error")
         * @param code the status code resulting from the HTTP request
         * @return the request error object
         */
        HttpRequestError create_error_from_http_status_code(unsigned int code);
    };

    ObjcHttpRequest::~ObjcHttpRequest() {
        // Cancel the request if executing
        cancel();
    }

    ObjcHttpRequest::ObjcHttpRequest(const HttpRequestParams& data, HttpRequestCallback callback) :
            _receive_timeout(data.receive_timeout), _request_timeout(data.request_timeout),
            _log_component(data.log_component), _proxy_address(data.proxy_address), _proxy_port(data.proxy_port),
            _common_name(data.common_name), _trusted_certs(data.trusted_certs), _verify_ssl(data.verify_ssl) {
        string url_prepended = data.url;
        // Check if http(s):// is prepended to the url
        if (url_prepended.find("http://") == string::npos && url_prepended.find("https://") == string::npos) {
            // Prepend http://
            url_prepended = "http://" + url_prepended;
        }

        NSString* url_objc = [NSString stringWithUTF8String:url_prepended.c_str()];

        _session_task = nullptr;
        // Initialize http request
        _request                 = [[NSMutableURLRequest alloc] init];
        _request.URL             = [NSURL URLWithString:url_objc];
        _request.cachePolicy     = NSURLRequestReloadIgnoringLocalAndRemoteCacheData;
        _request.timeoutInterval = data.receive_timeout;

        std::unique_lock<std::mutex> executing_lock(_executing_mutex);

        _state = HTTP_REQUEST_STATE_EXECUTING;

        for (auto header_item : data.headers) {
            NSString* name  = [NSString stringWithUTF8String:header_item.first.c_str()];
            NSString* value = [NSString stringWithUTF8String:header_item.second.c_str()];

            [_request setValue:value forHTTPHeaderField:name];
        }
        _request.HTTPMethod      = [NSString stringWithUTF8String:data.method.c_str()];

        bool file_upload = false;

        if (!data.body.empty() && data.file == nullptr) {
            _request.HTTPBody = [NSData dataWithBytes:data.body.data() length:data.body.size()];
            // Check if file should be uploaded
        } else if (data.file != nullptr) {
            Date date;
            // Get unique boundary based on the time
            size_t boundary_number = static_cast<size_t>(date.get_time().time_since_epoch().count());

            NSString *boundary     = [NSString stringWithFormat:@"---------------------------%zu", boundary_number];
            NSString *content_type = [NSString stringWithFormat:@"multipart/form-data; boundary=%@", boundary];
            [_request setValue:content_type forHTTPHeaderField:[NSString stringWithUTF8String:HTTP_HEADER_CONTENT_TYPE]];

            NSMutableData *file_body = [NSMutableData data];
            [file_body appendData:[[NSString stringWithFormat:@"\r\n--%@rn", boundary] dataUsingEncoding:NSUTF8StringEncoding]];
            [file_body appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"file\"; filename=\"file.fw\"\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
            [file_body appendData:[[NSString stringWithFormat:@"Content-Type: application/octet-stream\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
            // Set the file data
            [file_body appendData:[NSData dataWithBytes:data.file->data.data() length:data.file->size]];
            [file_body appendData:[[NSString stringWithFormat:@"\r\n--%@--\r\n",boundary] dataUsingEncoding:NSUTF8StringEncoding]];

            _request.HTTPBody = file_body;

            file_upload = true;
        }

        if (!file_upload) {
            HUE_LOG << _log_component <<  HUE_DEBUG << "ObjcHttpRequest: url: " << data.url << ", method: " << data.method << ", body: " << data.body << HUE_ENDL;
        } else {
            HUE_LOG << _log_component <<  HUE_DEBUG << "ObjcHttpRequest: (file upload) url: " << data.url << ", method: " << data.method << ", file size: " << static_cast<int64_t>(data.file->size) << HUE_ENDL;
        }

        start_execution_thread(callback);
    }

    void ObjcHttpRequest::cancel() {
        std::unique_lock<std::mutex> execution_lock(_executing_mutex);

        if (_state == HTTP_REQUEST_STATE_EXECUTING) {
            _state = HTTP_REQUEST_STATE_CANCELLED;

            [_session_task cancel];

            execution_lock.unlock();
        } else if (_state == HTTP_REQUEST_STATE_CANCELLED) {
            execution_lock.unlock();
        }

        if (_executing_thread != nullptr) {
            // Wait until the executing thread is finished
            _executing_thread->join();
        }
    }

    void ObjcHttpRequest::start_execution_thread(HttpRequestCallback callback) {
        ThreadTask task = [this, callback] () -> void {
            __block HttpResponse http_response;
            __block HttpRequestError http_error;

            std::unique_lock<std::mutex> execution_lock(_executing_mutex);

            if (_state != HTTP_REQUEST_STATE_CANCELLED) {
                atomic<bool> callback_called(false);
                // Create pointer to the callback_called which is capturable in the objective-c block. callback_called can only be captured in c++ lambda's without the __block keyword
                __block atomic<bool>* callback_called_ptr = &callback_called;

                // Create capturable condition variable
                __block condition_variable *execution_condition_ptr = new condition_variable();

                HUE_LOG << _log_component << HUE_DEBUG << "ObjcHttpRequest: performing request, receive timeout: " << _receive_timeout << ", request timeout: " << _request_timeout << HUE_ENDL;

                // Get url session
                NSURLSession * session = get_url_session(_proxy_address, _proxy_port);

                _session_task = [session dataTaskWithRequest:_request completionHandler:^(NSData *data, NSURLResponse *response, NSError *error) {
                    std::unique_lock<std::mutex> complition_handler_execution_lock(_executing_mutex);

                    if (error == nil) {
                        NSHTTPURLResponse * http_response_objc = (NSHTTPURLResponse *) response;
                        // Response headers
                        NSDictionary *header_fields = [http_response_objc allHeaderFields];

                        unsigned int status_code = static_cast<unsigned int>([http_response_objc statusCode]);

                        // Get body string
                        string body(static_cast<const char *>([data bytes]), data.length);

                        HUE_LOG << _log_component << HUE_DEBUG << "ObjcHttpRequest: response body: " << body << ", with code: " << to_string(status_code) << HUE_ENDL;

                        http_response = HttpResponse(status_code, body.c_str(), data.length);
                        // Set header fields
                        if (header_fields != nullptr) {
                            for (NSString *field in header_fields) {
                                string name = [field UTF8String];
                                string value = [[header_fields objectForKey:field] UTF8String];
                                http_response.add_header_field(name.c_str(), value.c_str());
                            }
                        }

                        http_response.set_certificate_chain(_certificate_chain);

                        http_error = create_error_from_http_status_code(status_code);
                    } else {
                        HttpRequestError::ErrorCode error_code = HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_UNDEFINED;

                        switch (error.code) {
                            case NSURLErrorTimedOut:
                                error_code = HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_TIMEOUT;
                                break;

                            case NSURLErrorCannotFindHost:
                                error_code = HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_CANNOT_RESOLVE_HOST;
                                break;

                            case NSURLErrorCannotConnectToHost:
                                error_code = HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_CANNOT_CONNECT;
                                break;

                            case NSURLErrorServerCertificateUntrusted:
                                error_code = HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_INVALID_CERTIFICATE;
                                break;

                            case NSURLErrorCancelled:
                                error_code = _ssl_error_cancellation ?
                                        HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_INVALID_CERTIFICATE :
                                        HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_CANCELED;
                                break;
                        }

                        string error_message = string([[[error userInfo] description] UTF8String]);

                        http_response = HttpResponse(0, "");
                        http_error = HttpRequestError(error_message, error_code);

                        HUE_LOG << _log_component << HUE_DEBUG << "ObjcHttpRequest: request failed, code: " << http_error.get_code() << ", message: " << http_error.get_message() << HUE_ENDL;
                    }

                    *callback_called_ptr = true;

                    execution_condition_ptr->notify_one();
                }];

                // Start execution in another thread
                [_session_task resume];

                // Wait for the callback
                execution_condition_ptr->wait(execution_lock, [&callback_called]() -> bool {
                    return callback_called;
                });

                delete (execution_condition_ptr);
            } else {
                // Cancelled
                http_error = HttpRequestError("", HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_CANCELED);
            }

            _state = HTTP_REQUEST_STATE_INITIAL;

            execution_lock.unlock();

            HUE_LOG << _log_component << HUE_DEBUG << "ObjcHttpRequest: calling callback" << HUE_ENDL;

            // Call the callback
            callback(http_error, http_response);
        };

        // Start thread with the task
        _executing_thread = unique_ptr<Thread>(new Thread(task));
    }

    HttpRequestError ObjcHttpRequest::create_error_from_http_status_code(unsigned int code) {
        switch (code) {
            case HTTP_RESPONSE_CODE_GATEWAY_TIMEOUT:
                return HttpRequestError("", HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_GATEWAY_TIMEOUT);
            default:
                return HttpRequestError("", HttpRequestError::ErrorCode::HTTP_REQUEST_ERROR_CODE_SUCCESS);
        }
    }

    /** ObjcHttpRequest::Impl */

    NSDictionary* ObjcHttpRequest::get_proxy_address_dictionary(string proxy_address, int proxy_port) {
        NSDictionary* proxy_dictionary = @{};

#if !TARGET_OS_WATCH
        NSString* n_proxy_address = [NSString stringWithUTF8String:proxy_address.c_str()];
        NSNumber* n_proxy_port    = [NSNumber numberWithInt: proxy_port];

        proxy_dictionary = @{
                (NSString *)kCFNetworkProxiesHTTPEnable : [NSNumber numberWithInt:1],
                (NSString *)kCFNetworkProxiesHTTPPort   : n_proxy_port,
                (NSString *)kCFNetworkProxiesHTTPProxy  : n_proxy_address,

                // kCFNetworkProxiesHTTPSEnable and friends cannot be used on iOS10
                // so we have to bypass the compilation error by providing the dict
                // strings directly
                @"HTTPSEnable" : [NSNumber numberWithInt:1],
                @"HTTPSPort"   : n_proxy_port,
                @"HTTPSProxy"  : n_proxy_address,
        };
#else
        (void)proxy_address;
        (void)proxy_port;
#endif

        return proxy_dictionary;
    }

    NSURLSession* ObjcHttpRequest::get_url_session(string proxy_address, int proxy_port) {
        NSURLSessionConfiguration *configuration = [NSURLSessionConfiguration ephemeralSessionConfiguration];

        NSDictionary* proxy_dict = nullptr;
        // Check if proxy has been set
        if (!proxy_address.empty()) {
            proxy_dict = get_proxy_address_dictionary(proxy_address, proxy_port);
            // Set proxy settings on the configuration
            configuration.connectionProxyDictionary = proxy_dict;
        }

        _session_delegate = [[HueURLSessionTaskDelegate alloc] initWithHttpRequest:this];

        return [NSURLSession sessionWithConfiguration:configuration delegate:_session_delegate delegateQueue:nil];
    }

    string ObjcHttpRequest::get_header_string(NSDictionary* headers) {
        string header_raw = "";

        for (NSString* key in headers) {
            NSString* value = [headers objectForKey:key];

            header_raw += string([key UTF8String]) + "=" + string([value UTF8String]) + "; ";
        }

        return header_raw;
    }

    ObjcHttpClient::ObjcHttpClient() {

    }

    ObjcHttpClient::~ObjcHttpClient() {

    }

    IHttpClient::Handle ObjcHttpClient::start_request(const HttpRequestParams &data, HttpRequestCallback callback) {
        if (!data.interface_name.empty()) {
            assert("Not supported on this platform");
        }
        return static_cast<Handle>(new ObjcHttpRequest(data, callback));
    }

    void ObjcHttpClient::stop_request(Handle handle) {
        delete static_cast<ObjcHttpRequest*>(handle);
    }
}  // namespace support

@implementation HueURLSessionTaskDelegate
- (void)URLSession:(NSURLSession * __unused)session task:(NSURLSessionTask * __unused)task willPerformHTTPRedirection:(NSHTTPURLResponse * __unused)response newRequest:(NSURLRequest * __unused)request completionHandler:(void (^)(NSURLRequest *_Nullable))completionHandler {
    completionHandler(nil);
}

- (id)initWithHttpRequest:(support::ObjcHttpRequest*) request {
    self = [super init];
    self.request = request;
    return self;
}

- (void)URLSession:(NSURLSession * __unused)session didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition, NSURLCredential *))completionHandler {
    if ([challenge.protectionSpace.authenticationMethod isEqualToString:NSURLAuthenticationMethodServerTrust]) {
        if (!self.request->verify_ssl()) {
            self.request->add_certificates_to_chain([self extract_certificates_from_trust:challenge.protectionSpace.serverTrust]);
            
            NSURLCredential *credential = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
            completionHandler(NSURLSessionAuthChallengeUseCredential, credential);
        } else if (!self.request->trusted_certs().empty()) {
            CFMutableArrayRef certificates = [self parse_certificates:self.request->trusted_certs()];

            bool cancel_request = true;
            if (CFArrayGetCount(certificates) > 0) {
                SecTrustRef host_trust = challenge.protectionSpace.serverTrust;
                if (!self.request->common_name().empty()) {
                    host_trust = [self change_host_for_trust:host_trust host:[NSString stringWithCString:self.request->common_name().c_str() encoding:[NSString defaultCStringEncoding]]];
                }

                NSArray *trusted_CAs = CFBridgingRelease(certificates);
                SecTrustSetAnchorCertificates(host_trust, (__bridge CFArrayRef) trusted_CAs);
                SecTrustSetAnchorCertificatesOnly(host_trust, true);

                SecTrustResultType result;
                OSStatus status = SecTrustEvaluate(host_trust, &result);
                if (status == errSecSuccess && (result == kSecTrustResultProceed || result == kSecTrustResultUnspecified)) {
                    self.request->add_certificates_to_chain([self extract_certificates_from_trust:host_trust]);

                    NSURLCredential *credential = [NSURLCredential credentialForTrust:host_trust];
                    [challenge.sender useCredential:credential forAuthenticationChallenge:challenge];
                    completionHandler(NSURLSessionAuthChallengeUseCredential, credential);
                    cancel_request = false;
                }

                if (!self.request->common_name().empty()) {
                    CFRelease(host_trust);
                }
            }

            if (cancel_request) {
                [[challenge sender] cancelAuthenticationChallenge:challenge];
                completionHandler(NSURLSessionAuthChallengeCancelAuthenticationChallenge, nil);
                self.request->set_ssl_error_cancellation(true);
            }
        }
    }
}

- (CFMutableArrayRef) parse_certificates:(const std::vector<std::string>&)pem_certificates
{
    CFMutableArrayRef certs = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

    for (auto&& pem_cert : pem_certificates) {
        auto der_cert = support::X509Certificate::get_pem_data(pem_cert);

        NSString *base64_encoded = [NSString stringWithCString:der_cert.c_str() encoding:[NSString defaultCStringEncoding]];
        NSData *data = [[NSData alloc] initWithBase64EncodedString:base64_encoded options:0];
        SecCertificateRef certificate = data ? SecCertificateCreateWithData(NULL, (__bridge CFDataRef) data) : nil;
        if (certificate) {
            CFArrayAppendValue(certs, certificate);
        }
    }

    return certs;
}

- (SecTrustRef) change_host_for_trust:(SecTrustRef)trust host:(NSString*) host
{
    CFIndex num_certs = SecTrustGetCertificateCount(trust);
    CFMutableArrayRef certs = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

    for (CFIndex idx = 0; idx < num_certs; ++idx)
    {
        SecCertificateRef cert = SecTrustGetCertificateAtIndex(trust, idx);
        CFArrayAppendValue(certs, cert);
    }

    SecPolicyRef policy = SecPolicyCreateSSL(true, (__bridge CFStringRef) host);
    SecTrustCreateWithCertificates(certs, policy, &trust);
    CFRelease(policy);
    CFRelease(certs);

    return trust;
}

- (std::vector<std::string>) extract_certificates_from_trust:(SecTrustRef)trust
{
    CFIndex num_certs = SecTrustGetCertificateCount(trust);
    std::vector<std::string> result;
    result.reserve(static_cast<size_t>(num_certs));
    for (CFIndex idx = 0; idx < num_certs; ++idx)
    {
        SecCertificateRef cert = SecTrustGetCertificateAtIndex(trust, idx);
        NSData *certificate_data = (NSData *)CFBridgingRelease(SecCertificateCopyData(cert));
        result.emplace_back(support::X509Certificate::der_to_pem(reinterpret_cast<const unsigned char*>([certificate_data bytes]), [certificate_data length]));
    }
    
    return result;
}

@end
