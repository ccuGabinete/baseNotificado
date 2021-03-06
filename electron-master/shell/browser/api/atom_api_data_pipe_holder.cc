// Copyright (c) 2019 GitHub, Inc.
// Use of this source code is governed by the MIT license that can be
// found in the LICENSE file.

#include "shell/browser/api/atom_api_data_pipe_holder.h"

#include <utility>
#include <vector>

#include "base/memory/weak_ptr.h"
#include "base/strings/string_number_conversions.h"
#include "mojo/public/cpp/system/data_pipe.h"
#include "mojo/public/cpp/system/simple_watcher.h"
#include "net/base/net_errors.h"
#include "shell/common/key_weak_map.h"
#include "shell/common/promise_util.h"

#include "shell/common/node_includes.h"

namespace electron {

namespace api {

namespace {

// Incremental ID.
int g_next_id = 0;

// Map that manages all the DataPipeHolder objects.
KeyWeakMap<std::string> g_weak_map;

// Utility class to read from data pipe.
class DataPipeReader {
 public:
  DataPipeReader(util::Promise<v8::Local<v8::Value>> promise,
                 network::mojom::DataPipeGetterPtr data_pipe_getter)
      : promise_(std::move(promise)),
        data_pipe_getter_(std::move(data_pipe_getter)),
        handle_watcher_(FROM_HERE,
                        mojo::SimpleWatcher::ArmingPolicy::MANUAL,
                        base::SequencedTaskRunnerHandle::Get()) {
    // Get a new data pipe and start.
    mojo::DataPipe data_pipe;
    data_pipe_getter_->Read(std::move(data_pipe.producer_handle),
                            base::BindOnce(&DataPipeReader::ReadCallback,
                                           weak_factory_.GetWeakPtr()));
    data_pipe_ = std::move(data_pipe.consumer_handle);
    handle_watcher_.Watch(data_pipe_.get(), MOJO_HANDLE_SIGNAL_READABLE,
                          base::BindRepeating(&DataPipeReader::OnHandleReadable,
                                              weak_factory_.GetWeakPtr()));
  }

  ~DataPipeReader() = default;

 private:
  // Callback invoked by DataPipeGetter::Read.
  void ReadCallback(int32_t status, uint64_t size) {
    if (status != net::OK) {
      OnFailure();
      return;
    }
    buffer_.resize(size);
    head_ = &buffer_.front();
    remaining_size_ = size;
    handle_watcher_.ArmOrNotify();
  }

  // Called by |handle_watcher_| when data is available or the pipe was closed,
  // and there's a pending Read() call.
  void OnHandleReadable(MojoResult result) {
    if (result != MOJO_RESULT_OK) {
      OnFailure();
      return;
    }

    // Read.
    uint32_t length = remaining_size_;
    result = data_pipe_->ReadData(head_, &length, MOJO_READ_DATA_FLAG_NONE);
    if (result == MOJO_RESULT_OK) {  // success
      remaining_size_ -= length;
      head_ += length;
      if (remaining_size_ == 0)
        OnSuccess();
    } else if (result == MOJO_RESULT_SHOULD_WAIT) {  // IO pending
      handle_watcher_.ArmOrNotify();
    } else {  // error
      OnFailure();
    }
  }

  void OnFailure() {
    promise_.RejectWithErrorMessage("Could not get blob data");
    delete this;
  }

  void OnSuccess() {
    // Pass the buffer to JS.
    //
    // Note that the lifetime of the native buffer belongs to us, and we will
    // free memory when JS buffer gets garbage collected.
    v8::Locker locker(promise_.isolate());
    v8::HandleScope handle_scope(promise_.isolate());
    v8::Local<v8::Value> buffer =
        node::Buffer::New(promise_.isolate(), &buffer_.front(), buffer_.size(),
                          &DataPipeReader::FreeBuffer, this)
            .ToLocalChecked();
    promise_.Resolve(buffer);

    // Destroy data pipe.
    handle_watcher_.Cancel();
    data_pipe_.reset();
    data_pipe_getter_ = nullptr;
  }

  static void FreeBuffer(char* data, void* self) {
    delete static_cast<DataPipeReader*>(self);
  }

  util::Promise<v8::Local<v8::Value>> promise_;

  network::mojom::DataPipeGetterPtr data_pipe_getter_;
  mojo::ScopedDataPipeConsumerHandle data_pipe_;
  mojo::SimpleWatcher handle_watcher_;

  // Stores read data.
  std::vector<char> buffer_;

  // The head of buffer.
  char* head_ = nullptr;

  // Remaining data to read.
  uint64_t remaining_size_ = 0;

  base::WeakPtrFactory<DataPipeReader> weak_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(DataPipeReader);
};

}  // namespace

gin::WrapperInfo DataPipeHolder::kWrapperInfo = {gin::kEmbedderNativeGin};

DataPipeHolder::DataPipeHolder(const network::DataElement& element)
    : id_(base::NumberToString(++g_next_id)),
      data_pipe_(element.CloneDataPipeGetter()) {}

DataPipeHolder::~DataPipeHolder() = default;

v8::Local<v8::Promise> DataPipeHolder::ReadAll(v8::Isolate* isolate) {
  util::Promise<v8::Local<v8::Value>> promise(isolate);
  v8::Local<v8::Promise> handle = promise.GetHandle();
  if (!data_pipe_) {
    promise.RejectWithErrorMessage("Could not get blob data");
    return handle;
  }

  new DataPipeReader(std::move(promise), std::move(data_pipe_));
  return handle;
}

// static
gin::Handle<DataPipeHolder> DataPipeHolder::Create(
    v8::Isolate* isolate,
    const network::DataElement& element) {
  auto handle = gin::CreateHandle(isolate, new DataPipeHolder(element));
  g_weak_map.Set(isolate, handle->id(),
                 handle->GetWrapper(isolate).ToLocalChecked());
  return handle;
}

// static
gin::Handle<DataPipeHolder> DataPipeHolder::From(v8::Isolate* isolate,
                                                 const std::string& id) {
  v8::MaybeLocal<v8::Object> object = g_weak_map.Get(isolate, id);
  if (!object.IsEmpty()) {
    gin::Handle<DataPipeHolder> handle;
    if (gin::ConvertFromV8(isolate, object.ToLocalChecked(), &handle))
      return handle;
  }
  return gin::Handle<DataPipeHolder>();
}

}  // namespace api

}  // namespace electron
