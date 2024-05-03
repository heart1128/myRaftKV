// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: kvServer.proto

#ifndef GOOGLE_PROTOBUF_INCLUDED_kvServer_2eproto
#define GOOGLE_PROTOBUF_INCLUDED_kvServer_2eproto

#include <limits>
#include <string>

#include <google/protobuf/port_def.inc>
#if PROTOBUF_VERSION < 3012000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers. Please update
#error your headers.
#endif
#if 3012004 < PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers. Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/port_undef.inc>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_table_driven.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/inlined_string_field.h>
#include <google/protobuf/metadata_lite.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>  // IWYU pragma: export
#include <google/protobuf/extension_set.h>  // IWYU pragma: export
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)
#include <google/protobuf/port_def.inc>
#define PROTOBUF_INTERNAL_EXPORT_kvServer_2eproto
PROTOBUF_NAMESPACE_OPEN
namespace internal {
class AnyMetadata;
}  // namespace internal
PROTOBUF_NAMESPACE_CLOSE

// Internal implementation detail -- do not use these members.
struct TableStruct_kvServer_2eproto {
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTableField entries[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::AuxillaryParseTableField aux[]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::ParseTable schema[1]
    PROTOBUF_SECTION_VARIABLE(protodesc_cold);
  static const ::PROTOBUF_NAMESPACE_ID::internal::FieldMetadata field_metadata[];
  static const ::PROTOBUF_NAMESPACE_ID::internal::SerializationTable serialization_table[];
  static const ::PROTOBUF_NAMESPACE_ID::uint32 offsets[];
};
extern const ::PROTOBUF_NAMESPACE_ID::internal::DescriptorTable descriptor_table_kvServer_2eproto;
namespace KVServer {
class KVMessage;
class KVMessageDefaultTypeInternal;
extern KVMessageDefaultTypeInternal _KVMessage_default_instance_;
}  // namespace KVServer
PROTOBUF_NAMESPACE_OPEN
template<> ::KVServer::KVMessage* Arena::CreateMaybeMessage<::KVServer::KVMessage>(Arena*);
PROTOBUF_NAMESPACE_CLOSE
namespace KVServer {

// ===================================================================

class KVMessage PROTOBUF_FINAL :
    public ::PROTOBUF_NAMESPACE_ID::Message /* @@protoc_insertion_point(class_definition:KVServer.KVMessage) */ {
 public:
  inline KVMessage() : KVMessage(nullptr) {};
  virtual ~KVMessage();

  KVMessage(const KVMessage& from);
  KVMessage(KVMessage&& from) noexcept
    : KVMessage() {
    *this = ::std::move(from);
  }

  inline KVMessage& operator=(const KVMessage& from) {
    CopyFrom(from);
    return *this;
  }
  inline KVMessage& operator=(KVMessage&& from) noexcept {
    if (GetArena() == from.GetArena()) {
      if (this != &from) InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::PROTOBUF_NAMESPACE_ID::Descriptor* GetDescriptor() {
    return GetMetadataStatic().descriptor;
  }
  static const ::PROTOBUF_NAMESPACE_ID::Reflection* GetReflection() {
    return GetMetadataStatic().reflection;
  }
  static const KVMessage& default_instance();

  static void InitAsDefaultInstance();  // FOR INTERNAL USE ONLY
  static inline const KVMessage* internal_default_instance() {
    return reinterpret_cast<const KVMessage*>(
               &_KVMessage_default_instance_);
  }
  static constexpr int kIndexInFileMessages =
    0;

  friend void swap(KVMessage& a, KVMessage& b) {
    a.Swap(&b);
  }
  inline void Swap(KVMessage* other) {
    if (other == this) return;
    if (GetArena() == other->GetArena()) {
      InternalSwap(other);
    } else {
      ::PROTOBUF_NAMESPACE_ID::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(KVMessage* other) {
    if (other == this) return;
    GOOGLE_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  inline KVMessage* New() const final {
    return CreateMaybeMessage<KVMessage>(nullptr);
  }

  KVMessage* New(::PROTOBUF_NAMESPACE_ID::Arena* arena) const final {
    return CreateMaybeMessage<KVMessage>(arena);
  }
  void CopyFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void MergeFrom(const ::PROTOBUF_NAMESPACE_ID::Message& from) final;
  void CopyFrom(const KVMessage& from);
  void MergeFrom(const KVMessage& from);
  PROTOBUF_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::PROTOBUF_NAMESPACE_ID::internal::ParseContext* ctx) final;
  ::PROTOBUF_NAMESPACE_ID::uint8* _InternalSerialize(
      ::PROTOBUF_NAMESPACE_ID::uint8* target, ::PROTOBUF_NAMESPACE_ID::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const final { return _cached_size_.Get(); }

  private:
  inline void SharedCtor();
  inline void SharedDtor();
  void SetCachedSize(int size) const final;
  void InternalSwap(KVMessage* other);
  friend class ::PROTOBUF_NAMESPACE_ID::internal::AnyMetadata;
  static ::PROTOBUF_NAMESPACE_ID::StringPiece FullMessageName() {
    return "KVServer.KVMessage";
  }
  protected:
  explicit KVMessage(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  private:
  static void ArenaDtor(void* object);
  inline void RegisterArenaDtor(::PROTOBUF_NAMESPACE_ID::Arena* arena);
  public:

  ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadata() const final;
  private:
  static ::PROTOBUF_NAMESPACE_ID::Metadata GetMetadataStatic() {
    ::PROTOBUF_NAMESPACE_ID::internal::AssignDescriptors(&::descriptor_table_kvServer_2eproto);
    return ::descriptor_table_kvServer_2eproto.file_level_metadata[kIndexInFileMessages];
  }

  public:

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  enum : int {
    kOperationFieldNumber = 1,
    kKeyFieldNumber = 2,
    kValueFieldNumber = 3,
    kClientIdFieldNumber = 4,
    kRequestIdFieldNumber = 5,
  };
  // bytes Operation = 1;
  void clear_operation();
  const std::string& operation() const;
  void set_operation(const std::string& value);
  void set_operation(std::string&& value);
  void set_operation(const char* value);
  void set_operation(const void* value, size_t size);
  std::string* mutable_operation();
  std::string* release_operation();
  void set_allocated_operation(std::string* operation);
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  std::string* unsafe_arena_release_operation();
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  void unsafe_arena_set_allocated_operation(
      std::string* operation);
  private:
  const std::string& _internal_operation() const;
  void _internal_set_operation(const std::string& value);
  std::string* _internal_mutable_operation();
  public:

  // bytes Key = 2;
  void clear_key();
  const std::string& key() const;
  void set_key(const std::string& value);
  void set_key(std::string&& value);
  void set_key(const char* value);
  void set_key(const void* value, size_t size);
  std::string* mutable_key();
  std::string* release_key();
  void set_allocated_key(std::string* key);
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  std::string* unsafe_arena_release_key();
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  void unsafe_arena_set_allocated_key(
      std::string* key);
  private:
  const std::string& _internal_key() const;
  void _internal_set_key(const std::string& value);
  std::string* _internal_mutable_key();
  public:

  // bytes Value = 3;
  void clear_value();
  const std::string& value() const;
  void set_value(const std::string& value);
  void set_value(std::string&& value);
  void set_value(const char* value);
  void set_value(const void* value, size_t size);
  std::string* mutable_value();
  std::string* release_value();
  void set_allocated_value(std::string* value);
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  std::string* unsafe_arena_release_value();
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  void unsafe_arena_set_allocated_value(
      std::string* value);
  private:
  const std::string& _internal_value() const;
  void _internal_set_value(const std::string& value);
  std::string* _internal_mutable_value();
  public:

  // bytes ClientId = 4;
  void clear_clientid();
  const std::string& clientid() const;
  void set_clientid(const std::string& value);
  void set_clientid(std::string&& value);
  void set_clientid(const char* value);
  void set_clientid(const void* value, size_t size);
  std::string* mutable_clientid();
  std::string* release_clientid();
  void set_allocated_clientid(std::string* clientid);
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  std::string* unsafe_arena_release_clientid();
  GOOGLE_PROTOBUF_RUNTIME_DEPRECATED("The unsafe_arena_ accessors for"
  "    string fields are deprecated and will be removed in a"
  "    future release.")
  void unsafe_arena_set_allocated_clientid(
      std::string* clientid);
  private:
  const std::string& _internal_clientid() const;
  void _internal_set_clientid(const std::string& value);
  std::string* _internal_mutable_clientid();
  public:

  // int32 RequestId = 5;
  void clear_requestid();
  ::PROTOBUF_NAMESPACE_ID::int32 requestid() const;
  void set_requestid(::PROTOBUF_NAMESPACE_ID::int32 value);
  private:
  ::PROTOBUF_NAMESPACE_ID::int32 _internal_requestid() const;
  void _internal_set_requestid(::PROTOBUF_NAMESPACE_ID::int32 value);
  public:

  // @@protoc_insertion_point(class_scope:KVServer.KVMessage)
 private:
  class _Internal;

  template <typename T> friend class ::PROTOBUF_NAMESPACE_ID::Arena::InternalHelper;
  typedef void InternalArenaConstructable_;
  typedef void DestructorSkippable_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr operation_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr key_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr value_;
  ::PROTOBUF_NAMESPACE_ID::internal::ArenaStringPtr clientid_;
  ::PROTOBUF_NAMESPACE_ID::int32 requestid_;
  mutable ::PROTOBUF_NAMESPACE_ID::internal::CachedSize _cached_size_;
  friend struct ::TableStruct_kvServer_2eproto;
};
// ===================================================================


// ===================================================================

#ifdef __GNUC__
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// KVMessage

// bytes Operation = 1;
inline void KVMessage::clear_operation() {
  operation_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& KVMessage::operation() const {
  // @@protoc_insertion_point(field_get:KVServer.KVMessage.Operation)
  return _internal_operation();
}
inline void KVMessage::set_operation(const std::string& value) {
  _internal_set_operation(value);
  // @@protoc_insertion_point(field_set:KVServer.KVMessage.Operation)
}
inline std::string* KVMessage::mutable_operation() {
  // @@protoc_insertion_point(field_mutable:KVServer.KVMessage.Operation)
  return _internal_mutable_operation();
}
inline const std::string& KVMessage::_internal_operation() const {
  return operation_.Get();
}
inline void KVMessage::_internal_set_operation(const std::string& value) {
  
  operation_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void KVMessage::set_operation(std::string&& value) {
  
  operation_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:KVServer.KVMessage.Operation)
}
inline void KVMessage::set_operation(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  operation_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:KVServer.KVMessage.Operation)
}
inline void KVMessage::set_operation(const void* value,
    size_t size) {
  
  operation_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:KVServer.KVMessage.Operation)
}
inline std::string* KVMessage::_internal_mutable_operation() {
  
  return operation_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* KVMessage::release_operation() {
  // @@protoc_insertion_point(field_release:KVServer.KVMessage.Operation)
  return operation_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void KVMessage::set_allocated_operation(std::string* operation) {
  if (operation != nullptr) {
    
  } else {
    
  }
  operation_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), operation,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:KVServer.KVMessage.Operation)
}
inline std::string* KVMessage::unsafe_arena_release_operation() {
  // @@protoc_insertion_point(field_unsafe_arena_release:KVServer.KVMessage.Operation)
  GOOGLE_DCHECK(GetArena() != nullptr);
  
  return operation_.UnsafeArenaRelease(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      GetArena());
}
inline void KVMessage::unsafe_arena_set_allocated_operation(
    std::string* operation) {
  GOOGLE_DCHECK(GetArena() != nullptr);
  if (operation != nullptr) {
    
  } else {
    
  }
  operation_.UnsafeArenaSetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      operation, GetArena());
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:KVServer.KVMessage.Operation)
}

// bytes Key = 2;
inline void KVMessage::clear_key() {
  key_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& KVMessage::key() const {
  // @@protoc_insertion_point(field_get:KVServer.KVMessage.Key)
  return _internal_key();
}
inline void KVMessage::set_key(const std::string& value) {
  _internal_set_key(value);
  // @@protoc_insertion_point(field_set:KVServer.KVMessage.Key)
}
inline std::string* KVMessage::mutable_key() {
  // @@protoc_insertion_point(field_mutable:KVServer.KVMessage.Key)
  return _internal_mutable_key();
}
inline const std::string& KVMessage::_internal_key() const {
  return key_.Get();
}
inline void KVMessage::_internal_set_key(const std::string& value) {
  
  key_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void KVMessage::set_key(std::string&& value) {
  
  key_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:KVServer.KVMessage.Key)
}
inline void KVMessage::set_key(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  key_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:KVServer.KVMessage.Key)
}
inline void KVMessage::set_key(const void* value,
    size_t size) {
  
  key_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:KVServer.KVMessage.Key)
}
inline std::string* KVMessage::_internal_mutable_key() {
  
  return key_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* KVMessage::release_key() {
  // @@protoc_insertion_point(field_release:KVServer.KVMessage.Key)
  return key_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void KVMessage::set_allocated_key(std::string* key) {
  if (key != nullptr) {
    
  } else {
    
  }
  key_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), key,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:KVServer.KVMessage.Key)
}
inline std::string* KVMessage::unsafe_arena_release_key() {
  // @@protoc_insertion_point(field_unsafe_arena_release:KVServer.KVMessage.Key)
  GOOGLE_DCHECK(GetArena() != nullptr);
  
  return key_.UnsafeArenaRelease(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      GetArena());
}
inline void KVMessage::unsafe_arena_set_allocated_key(
    std::string* key) {
  GOOGLE_DCHECK(GetArena() != nullptr);
  if (key != nullptr) {
    
  } else {
    
  }
  key_.UnsafeArenaSetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      key, GetArena());
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:KVServer.KVMessage.Key)
}

// bytes Value = 3;
inline void KVMessage::clear_value() {
  value_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& KVMessage::value() const {
  // @@protoc_insertion_point(field_get:KVServer.KVMessage.Value)
  return _internal_value();
}
inline void KVMessage::set_value(const std::string& value) {
  _internal_set_value(value);
  // @@protoc_insertion_point(field_set:KVServer.KVMessage.Value)
}
inline std::string* KVMessage::mutable_value() {
  // @@protoc_insertion_point(field_mutable:KVServer.KVMessage.Value)
  return _internal_mutable_value();
}
inline const std::string& KVMessage::_internal_value() const {
  return value_.Get();
}
inline void KVMessage::_internal_set_value(const std::string& value) {
  
  value_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void KVMessage::set_value(std::string&& value) {
  
  value_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:KVServer.KVMessage.Value)
}
inline void KVMessage::set_value(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  value_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:KVServer.KVMessage.Value)
}
inline void KVMessage::set_value(const void* value,
    size_t size) {
  
  value_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:KVServer.KVMessage.Value)
}
inline std::string* KVMessage::_internal_mutable_value() {
  
  return value_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* KVMessage::release_value() {
  // @@protoc_insertion_point(field_release:KVServer.KVMessage.Value)
  return value_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void KVMessage::set_allocated_value(std::string* value) {
  if (value != nullptr) {
    
  } else {
    
  }
  value_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:KVServer.KVMessage.Value)
}
inline std::string* KVMessage::unsafe_arena_release_value() {
  // @@protoc_insertion_point(field_unsafe_arena_release:KVServer.KVMessage.Value)
  GOOGLE_DCHECK(GetArena() != nullptr);
  
  return value_.UnsafeArenaRelease(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      GetArena());
}
inline void KVMessage::unsafe_arena_set_allocated_value(
    std::string* value) {
  GOOGLE_DCHECK(GetArena() != nullptr);
  if (value != nullptr) {
    
  } else {
    
  }
  value_.UnsafeArenaSetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      value, GetArena());
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:KVServer.KVMessage.Value)
}

// bytes ClientId = 4;
inline void KVMessage::clear_clientid() {
  clientid_.ClearToEmpty(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline const std::string& KVMessage::clientid() const {
  // @@protoc_insertion_point(field_get:KVServer.KVMessage.ClientId)
  return _internal_clientid();
}
inline void KVMessage::set_clientid(const std::string& value) {
  _internal_set_clientid(value);
  // @@protoc_insertion_point(field_set:KVServer.KVMessage.ClientId)
}
inline std::string* KVMessage::mutable_clientid() {
  // @@protoc_insertion_point(field_mutable:KVServer.KVMessage.ClientId)
  return _internal_mutable_clientid();
}
inline const std::string& KVMessage::_internal_clientid() const {
  return clientid_.Get();
}
inline void KVMessage::_internal_set_clientid(const std::string& value) {
  
  clientid_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), value, GetArena());
}
inline void KVMessage::set_clientid(std::string&& value) {
  
  clientid_.Set(
    &::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::move(value), GetArena());
  // @@protoc_insertion_point(field_set_rvalue:KVServer.KVMessage.ClientId)
}
inline void KVMessage::set_clientid(const char* value) {
  GOOGLE_DCHECK(value != nullptr);
  
  clientid_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(value),
              GetArena());
  // @@protoc_insertion_point(field_set_char:KVServer.KVMessage.ClientId)
}
inline void KVMessage::set_clientid(const void* value,
    size_t size) {
  
  clientid_.Set(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), ::std::string(
      reinterpret_cast<const char*>(value), size), GetArena());
  // @@protoc_insertion_point(field_set_pointer:KVServer.KVMessage.ClientId)
}
inline std::string* KVMessage::_internal_mutable_clientid() {
  
  return clientid_.Mutable(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline std::string* KVMessage::release_clientid() {
  // @@protoc_insertion_point(field_release:KVServer.KVMessage.ClientId)
  return clientid_.Release(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), GetArena());
}
inline void KVMessage::set_allocated_clientid(std::string* clientid) {
  if (clientid != nullptr) {
    
  } else {
    
  }
  clientid_.SetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(), clientid,
      GetArena());
  // @@protoc_insertion_point(field_set_allocated:KVServer.KVMessage.ClientId)
}
inline std::string* KVMessage::unsafe_arena_release_clientid() {
  // @@protoc_insertion_point(field_unsafe_arena_release:KVServer.KVMessage.ClientId)
  GOOGLE_DCHECK(GetArena() != nullptr);
  
  return clientid_.UnsafeArenaRelease(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      GetArena());
}
inline void KVMessage::unsafe_arena_set_allocated_clientid(
    std::string* clientid) {
  GOOGLE_DCHECK(GetArena() != nullptr);
  if (clientid != nullptr) {
    
  } else {
    
  }
  clientid_.UnsafeArenaSetAllocated(&::PROTOBUF_NAMESPACE_ID::internal::GetEmptyStringAlreadyInited(),
      clientid, GetArena());
  // @@protoc_insertion_point(field_unsafe_arena_set_allocated:KVServer.KVMessage.ClientId)
}

// int32 RequestId = 5;
inline void KVMessage::clear_requestid() {
  requestid_ = 0;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 KVMessage::_internal_requestid() const {
  return requestid_;
}
inline ::PROTOBUF_NAMESPACE_ID::int32 KVMessage::requestid() const {
  // @@protoc_insertion_point(field_get:KVServer.KVMessage.RequestId)
  return _internal_requestid();
}
inline void KVMessage::_internal_set_requestid(::PROTOBUF_NAMESPACE_ID::int32 value) {
  
  requestid_ = value;
}
inline void KVMessage::set_requestid(::PROTOBUF_NAMESPACE_ID::int32 value) {
  _internal_set_requestid(value);
  // @@protoc_insertion_point(field_set:KVServer.KVMessage.RequestId)
}

#ifdef __GNUC__
  #pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)

}  // namespace KVServer

// @@protoc_insertion_point(global_scope)

#include <google/protobuf/port_undef.inc>
#endif  // GOOGLE_PROTOBUF_INCLUDED_GOOGLE_PROTOBUF_INCLUDED_kvServer_2eproto