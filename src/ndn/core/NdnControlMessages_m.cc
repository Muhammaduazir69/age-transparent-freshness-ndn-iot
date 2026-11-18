//
// Generated file, do not edit! Created by opp_msgtool 6.4 from ndn/core/NdnControlMessages.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "NdnControlMessages_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp


template<typename T>
std::string toStringIfPrintable(const T& t) {
    if constexpr (omnetpp::internal::is_printable<T>::value) {
        std::ostringstream os;
        os << t;
        return os.str();
    }
    return omnetpp::cClassDescriptor::UNPRINTABLE;
}

template<typename T>
bool fromStringIfExtractable(T& t, const char *s) {
    if constexpr (omnetpp::internal::is_extractable<T>::value) {
        std::istringstream is(s);
        is >> t;
        return true;
    }
    return false;
}

namespace ndniot {

Register_Enum(ndniot::NDNControlMessageType, (ndniot::NDNControlMessageType::PIT_INSERT_REQUEST, ndniot::NDNControlMessageType::PIT_INSERT_RESPONSE, ndniot::NDNControlMessageType::PIT_LOOKUP_REQUEST, ndniot::NDNControlMessageType::PIT_LOOKUP_RESPONSE, ndniot::NDNControlMessageType::PIT_SATISFY_REQUEST, ndniot::NDNControlMessageType::PIT_SATISFY_RESPONSE, ndniot::NDNControlMessageType::FIB_LOOKUP_REQUEST, ndniot::NDNControlMessageType::FIB_LOOKUP_RESPONSE, ndniot::NDNControlMessageType::FIB_ADD_ROUTE_REQUEST, ndniot::NDNControlMessageType::FIB_ADD_ROUTE_RESPONSE, ndniot::NDNControlMessageType::CS_LOOKUP_REQUEST, ndniot::NDNControlMessageType::CS_LOOKUP_RESPONSE, ndniot::NDNControlMessageType::CS_INSERT_REQUEST, ndniot::NDNControlMessageType::CS_INSERT_RESPONSE));

Register_Class(NDNControlMessage)

NDNControlMessage::NDNControlMessage(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

NDNControlMessage::NDNControlMessage(const NDNControlMessage& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

NDNControlMessage::~NDNControlMessage()
{
}

NDNControlMessage& NDNControlMessage::operator=(const NDNControlMessage& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void NDNControlMessage::copy(const NDNControlMessage& other)
{
    this->messageType = other.messageType;
    this->name = other.name;
    this->transactionId = other.transactionId;
}

void NDNControlMessage::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->messageType);
    doParsimPacking(b,this->name);
    doParsimPacking(b,this->transactionId);
}

void NDNControlMessage::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->messageType);
    doParsimUnpacking(b,this->name);
    doParsimUnpacking(b,this->transactionId);
}

int NDNControlMessage::getMessageType() const
{
    return this->messageType;
}

void NDNControlMessage::setMessageType(int messageType)
{
    this->messageType = messageType;
}

const char * NDNControlMessage::getName() const
{
    return this->name.c_str();
}

void NDNControlMessage::setName(const char * name)
{
    this->name = name;
}

int NDNControlMessage::getTransactionId() const
{
    return this->transactionId;
}

void NDNControlMessage::setTransactionId(int transactionId)
{
    this->transactionId = transactionId;
}

class NDNControlMessageDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_messageType,
        FIELD_name,
        FIELD_transactionId,
    };
  public:
    NDNControlMessageDescriptor();
    virtual ~NDNControlMessageDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(NDNControlMessageDescriptor)

NDNControlMessageDescriptor::NDNControlMessageDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::NDNControlMessage)), "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

NDNControlMessageDescriptor::~NDNControlMessageDescriptor()
{
    delete[] propertyNames;
}

bool NDNControlMessageDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<NDNControlMessage *>(obj)!=nullptr;
}

const char **NDNControlMessageDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *NDNControlMessageDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string NDNControlMessageDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void NDNControlMessageDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int NDNControlMessageDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int NDNControlMessageDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_messageType
        FD_ISEDITABLE,    // FIELD_name
        FD_ISEDITABLE,    // FIELD_transactionId
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *NDNControlMessageDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "messageType",
        "name",
        "transactionId",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int NDNControlMessageDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "messageType") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "name") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "transactionId") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *NDNControlMessageDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_messageType
        "string",    // FIELD_name
        "int",    // FIELD_transactionId
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **NDNControlMessageDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_messageType: {
            static const char *names[] = { "enum", "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *NDNControlMessageDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_messageType:
            if (!strcmp(propertyName, "enum")) return "NDNControlMessageType";
            if (!strcmp(propertyName, "enum")) return "ndniot::NDNControlMessageType";
            return nullptr;
        default: return nullptr;
    }
}

int NDNControlMessageDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void NDNControlMessageDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'NDNControlMessage'", field);
    }
}

const char *NDNControlMessageDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string NDNControlMessageDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    switch (field) {
        case FIELD_messageType: return enum2string(static_cast<int>(pp->getMessageType()), "ndniot::NDNControlMessageType");
        case FIELD_name: return oppstring2string(pp->getName());
        case FIELD_transactionId: return long2string(pp->getTransactionId());
        default: return "";
    }
}

void NDNControlMessageDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    switch (field) {
        case FIELD_messageType: pp->setMessageType((ndniot::NDNControlMessageType)string2enum(value, "ndniot::NDNControlMessageType")); break;
        case FIELD_name: pp->setName((value)); break;
        case FIELD_transactionId: pp->setTransactionId(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NDNControlMessage'", field);
    }
}

omnetpp::cValue NDNControlMessageDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    switch (field) {
        case FIELD_messageType: return pp->getMessageType();
        case FIELD_name: return pp->getName();
        case FIELD_transactionId: return pp->getTransactionId();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'NDNControlMessage' as cValue -- field index out of range?", field);
    }
}

void NDNControlMessageDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    switch (field) {
        case FIELD_messageType: pp->setMessageType((ndniot::NDNControlMessageType)value.intValue()); break;
        case FIELD_name: pp->setName(value.stringValue()); break;
        case FIELD_transactionId: pp->setTransactionId(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NDNControlMessage'", field);
    }
}

const char *NDNControlMessageDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr NDNControlMessageDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void NDNControlMessageDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    NDNControlMessage *pp = omnetpp::fromAnyPtr<NDNControlMessage>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NDNControlMessage'", field);
    }
}

Register_Class(PITInsertRequest)

PITInsertRequest::PITInsertRequest(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(PIT_INSERT_REQUEST);

}

PITInsertRequest::PITInsertRequest(const PITInsertRequest& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

PITInsertRequest::~PITInsertRequest()
{
}

PITInsertRequest& PITInsertRequest::operator=(const PITInsertRequest& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void PITInsertRequest::copy(const PITInsertRequest& other)
{
    this->inFace = other.inFace;
    this->nonce = other.nonce;
    this->interestLifetime = other.interestLifetime;
}

void PITInsertRequest::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->inFace);
    doParsimPacking(b,this->nonce);
    doParsimPacking(b,this->interestLifetime);
}

void PITInsertRequest::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->inFace);
    doParsimUnpacking(b,this->nonce);
    doParsimUnpacking(b,this->interestLifetime);
}

int PITInsertRequest::getInFace() const
{
    return this->inFace;
}

void PITInsertRequest::setInFace(int inFace)
{
    this->inFace = inFace;
}

int PITInsertRequest::getNonce() const
{
    return this->nonce;
}

void PITInsertRequest::setNonce(int nonce)
{
    this->nonce = nonce;
}

double PITInsertRequest::getInterestLifetime() const
{
    return this->interestLifetime;
}

void PITInsertRequest::setInterestLifetime(double interestLifetime)
{
    this->interestLifetime = interestLifetime;
}

class PITInsertRequestDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_inFace,
        FIELD_nonce,
        FIELD_interestLifetime,
    };
  public:
    PITInsertRequestDescriptor();
    virtual ~PITInsertRequestDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(PITInsertRequestDescriptor)

PITInsertRequestDescriptor::PITInsertRequestDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::PITInsertRequest)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

PITInsertRequestDescriptor::~PITInsertRequestDescriptor()
{
    delete[] propertyNames;
}

bool PITInsertRequestDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<PITInsertRequest *>(obj)!=nullptr;
}

const char **PITInsertRequestDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *PITInsertRequestDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string PITInsertRequestDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void PITInsertRequestDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int PITInsertRequestDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int PITInsertRequestDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_inFace
        FD_ISEDITABLE,    // FIELD_nonce
        FD_ISEDITABLE,    // FIELD_interestLifetime
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *PITInsertRequestDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "inFace",
        "nonce",
        "interestLifetime",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int PITInsertRequestDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "inFace") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "nonce") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "interestLifetime") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *PITInsertRequestDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_inFace
        "int",    // FIELD_nonce
        "double",    // FIELD_interestLifetime
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **PITInsertRequestDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *PITInsertRequestDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int PITInsertRequestDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void PITInsertRequestDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'PITInsertRequest'", field);
    }
}

const char *PITInsertRequestDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string PITInsertRequestDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: return long2string(pp->getInFace());
        case FIELD_nonce: return long2string(pp->getNonce());
        case FIELD_interestLifetime: return double2string(pp->getInterestLifetime());
        default: return "";
    }
}

void PITInsertRequestDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: pp->setInFace(string2long(value)); break;
        case FIELD_nonce: pp->setNonce(string2long(value)); break;
        case FIELD_interestLifetime: pp->setInterestLifetime(string2double(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITInsertRequest'", field);
    }
}

omnetpp::cValue PITInsertRequestDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: return pp->getInFace();
        case FIELD_nonce: return pp->getNonce();
        case FIELD_interestLifetime: return pp->getInterestLifetime();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'PITInsertRequest' as cValue -- field index out of range?", field);
    }
}

void PITInsertRequestDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: pp->setInFace(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_nonce: pp->setNonce(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_interestLifetime: pp->setInterestLifetime(value.doubleValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITInsertRequest'", field);
    }
}

const char *PITInsertRequestDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr PITInsertRequestDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void PITInsertRequestDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    PITInsertRequest *pp = omnetpp::fromAnyPtr<PITInsertRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITInsertRequest'", field);
    }
}

Register_Class(PITInsertResponse)

PITInsertResponse::PITInsertResponse(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(PIT_INSERT_RESPONSE);

}

PITInsertResponse::PITInsertResponse(const PITInsertResponse& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

PITInsertResponse::~PITInsertResponse()
{
    delete [] this->outFaces;
}

PITInsertResponse& PITInsertResponse::operator=(const PITInsertResponse& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void PITInsertResponse::copy(const PITInsertResponse& other)
{
    this->success = other.success;
    this->aggregated = other.aggregated;
    delete [] this->outFaces;
    this->outFaces = (other.outFaces_arraysize==0) ? nullptr : new int[other.outFaces_arraysize];
    outFaces_arraysize = other.outFaces_arraysize;
    for (size_t i = 0; i < outFaces_arraysize; i++) {
        this->outFaces[i] = other.outFaces[i];
    }
}

void PITInsertResponse::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->success);
    doParsimPacking(b,this->aggregated);
    b->pack(outFaces_arraysize);
    doParsimArrayPacking(b,this->outFaces,outFaces_arraysize);
}

void PITInsertResponse::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->success);
    doParsimUnpacking(b,this->aggregated);
    delete [] this->outFaces;
    b->unpack(outFaces_arraysize);
    if (outFaces_arraysize == 0) {
        this->outFaces = nullptr;
    } else {
        this->outFaces = new int[outFaces_arraysize];
        doParsimArrayUnpacking(b,this->outFaces,outFaces_arraysize);
    }
}

bool PITInsertResponse::getSuccess() const
{
    return this->success;
}

void PITInsertResponse::setSuccess(bool success)
{
    this->success = success;
}

bool PITInsertResponse::getAggregated() const
{
    return this->aggregated;
}

void PITInsertResponse::setAggregated(bool aggregated)
{
    this->aggregated = aggregated;
}

size_t PITInsertResponse::getOutFacesArraySize() const
{
    return outFaces_arraysize;
}

int PITInsertResponse::getOutFaces(size_t k) const
{
    if (k >= outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    return this->outFaces[k];
}

void PITInsertResponse::setOutFacesArraySize(size_t newSize)
{
    int *outFaces2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = outFaces_arraysize < newSize ? outFaces_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        outFaces2[i] = this->outFaces[i];
    for (size_t i = minSize; i < newSize; i++)
        outFaces2[i] = 0;
    delete [] this->outFaces;
    this->outFaces = outFaces2;
    outFaces_arraysize = newSize;
}

void PITInsertResponse::setOutFaces(size_t k, int outFaces)
{
    if (k >= outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    this->outFaces[k] = outFaces;
}

void PITInsertResponse::insertOutFaces(size_t k, int outFaces)
{
    if (k > outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    size_t newSize = outFaces_arraysize + 1;
    int *outFaces2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        outFaces2[i] = this->outFaces[i];
    outFaces2[k] = outFaces;
    for (i = k + 1; i < newSize; i++)
        outFaces2[i] = this->outFaces[i-1];
    delete [] this->outFaces;
    this->outFaces = outFaces2;
    outFaces_arraysize = newSize;
}

void PITInsertResponse::appendOutFaces(int outFaces)
{
    insertOutFaces(outFaces_arraysize, outFaces);
}

void PITInsertResponse::eraseOutFaces(size_t k)
{
    if (k >= outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    size_t newSize = outFaces_arraysize - 1;
    int *outFaces2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        outFaces2[i] = this->outFaces[i];
    for (i = k; i < newSize; i++)
        outFaces2[i] = this->outFaces[i+1];
    delete [] this->outFaces;
    this->outFaces = outFaces2;
    outFaces_arraysize = newSize;
}

class PITInsertResponseDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_success,
        FIELD_aggregated,
        FIELD_outFaces,
    };
  public:
    PITInsertResponseDescriptor();
    virtual ~PITInsertResponseDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(PITInsertResponseDescriptor)

PITInsertResponseDescriptor::PITInsertResponseDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::PITInsertResponse)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

PITInsertResponseDescriptor::~PITInsertResponseDescriptor()
{
    delete[] propertyNames;
}

bool PITInsertResponseDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<PITInsertResponse *>(obj)!=nullptr;
}

const char **PITInsertResponseDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *PITInsertResponseDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string PITInsertResponseDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void PITInsertResponseDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int PITInsertResponseDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int PITInsertResponseDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_success
        FD_ISEDITABLE,    // FIELD_aggregated
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_outFaces
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *PITInsertResponseDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "success",
        "aggregated",
        "outFaces",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int PITInsertResponseDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "success") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "aggregated") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "outFaces") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *PITInsertResponseDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_success
        "bool",    // FIELD_aggregated
        "int",    // FIELD_outFaces
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **PITInsertResponseDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *PITInsertResponseDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int PITInsertResponseDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    switch (field) {
        case FIELD_outFaces: return pp->getOutFacesArraySize();
        default: return 0;
    }
}

void PITInsertResponseDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    switch (field) {
        case FIELD_outFaces: pp->setOutFacesArraySize(size); break;
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'PITInsertResponse'", field);
    }
}

const char *PITInsertResponseDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string PITInsertResponseDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: return bool2string(pp->getSuccess());
        case FIELD_aggregated: return bool2string(pp->getAggregated());
        case FIELD_outFaces: return long2string(pp->getOutFaces(i));
        default: return "";
    }
}

void PITInsertResponseDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: pp->setSuccess(string2bool(value)); break;
        case FIELD_aggregated: pp->setAggregated(string2bool(value)); break;
        case FIELD_outFaces: pp->setOutFaces(i,string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITInsertResponse'", field);
    }
}

omnetpp::cValue PITInsertResponseDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: return pp->getSuccess();
        case FIELD_aggregated: return pp->getAggregated();
        case FIELD_outFaces: return pp->getOutFaces(i);
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'PITInsertResponse' as cValue -- field index out of range?", field);
    }
}

void PITInsertResponseDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: pp->setSuccess(value.boolValue()); break;
        case FIELD_aggregated: pp->setAggregated(value.boolValue()); break;
        case FIELD_outFaces: pp->setOutFaces(i,omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITInsertResponse'", field);
    }
}

const char *PITInsertResponseDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr PITInsertResponseDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void PITInsertResponseDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    PITInsertResponse *pp = omnetpp::fromAnyPtr<PITInsertResponse>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITInsertResponse'", field);
    }
}

Register_Class(PITLookupRequest)

PITLookupRequest::PITLookupRequest(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(PIT_LOOKUP_REQUEST);
}

PITLookupRequest::PITLookupRequest(const PITLookupRequest& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

PITLookupRequest::~PITLookupRequest()
{
}

PITLookupRequest& PITLookupRequest::operator=(const PITLookupRequest& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void PITLookupRequest::copy(const PITLookupRequest& other)
{
}

void PITLookupRequest::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
}

void PITLookupRequest::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
}

class PITLookupRequestDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
    };
  public:
    PITLookupRequestDescriptor();
    virtual ~PITLookupRequestDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(PITLookupRequestDescriptor)

PITLookupRequestDescriptor::PITLookupRequestDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::PITLookupRequest)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

PITLookupRequestDescriptor::~PITLookupRequestDescriptor()
{
    delete[] propertyNames;
}

bool PITLookupRequestDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<PITLookupRequest *>(obj)!=nullptr;
}

const char **PITLookupRequestDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *PITLookupRequestDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string PITLookupRequestDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void PITLookupRequestDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int PITLookupRequestDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 0+base->getFieldCount() : 0;
}

unsigned int PITLookupRequestDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    return 0;
}

const char *PITLookupRequestDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

int PITLookupRequestDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->findField(fieldName) : -1;
}

const char *PITLookupRequestDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

const char **PITLookupRequestDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *PITLookupRequestDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int PITLookupRequestDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void PITLookupRequestDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'PITLookupRequest'", field);
    }
}

const char *PITLookupRequestDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string PITLookupRequestDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    switch (field) {
        default: return "";
    }
}

void PITLookupRequestDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITLookupRequest'", field);
    }
}

omnetpp::cValue PITLookupRequestDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'PITLookupRequest' as cValue -- field index out of range?", field);
    }
}

void PITLookupRequestDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITLookupRequest'", field);
    }
}

const char *PITLookupRequestDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

omnetpp::any_ptr PITLookupRequestDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void PITLookupRequestDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    PITLookupRequest *pp = omnetpp::fromAnyPtr<PITLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITLookupRequest'", field);
    }
}

Register_Class(PITLookupResponse)

PITLookupResponse::PITLookupResponse(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(PIT_LOOKUP_RESPONSE);

}

PITLookupResponse::PITLookupResponse(const PITLookupResponse& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

PITLookupResponse::~PITLookupResponse()
{
    delete [] this->inFaces;
    delete [] this->outFaces;
}

PITLookupResponse& PITLookupResponse::operator=(const PITLookupResponse& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void PITLookupResponse::copy(const PITLookupResponse& other)
{
    this->found = other.found;
    delete [] this->inFaces;
    this->inFaces = (other.inFaces_arraysize==0) ? nullptr : new int[other.inFaces_arraysize];
    inFaces_arraysize = other.inFaces_arraysize;
    for (size_t i = 0; i < inFaces_arraysize; i++) {
        this->inFaces[i] = other.inFaces[i];
    }
    delete [] this->outFaces;
    this->outFaces = (other.outFaces_arraysize==0) ? nullptr : new int[other.outFaces_arraysize];
    outFaces_arraysize = other.outFaces_arraysize;
    for (size_t i = 0; i < outFaces_arraysize; i++) {
        this->outFaces[i] = other.outFaces[i];
    }
}

void PITLookupResponse::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->found);
    b->pack(inFaces_arraysize);
    doParsimArrayPacking(b,this->inFaces,inFaces_arraysize);
    b->pack(outFaces_arraysize);
    doParsimArrayPacking(b,this->outFaces,outFaces_arraysize);
}

void PITLookupResponse::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->found);
    delete [] this->inFaces;
    b->unpack(inFaces_arraysize);
    if (inFaces_arraysize == 0) {
        this->inFaces = nullptr;
    } else {
        this->inFaces = new int[inFaces_arraysize];
        doParsimArrayUnpacking(b,this->inFaces,inFaces_arraysize);
    }
    delete [] this->outFaces;
    b->unpack(outFaces_arraysize);
    if (outFaces_arraysize == 0) {
        this->outFaces = nullptr;
    } else {
        this->outFaces = new int[outFaces_arraysize];
        doParsimArrayUnpacking(b,this->outFaces,outFaces_arraysize);
    }
}

bool PITLookupResponse::getFound() const
{
    return this->found;
}

void PITLookupResponse::setFound(bool found)
{
    this->found = found;
}

size_t PITLookupResponse::getInFacesArraySize() const
{
    return inFaces_arraysize;
}

int PITLookupResponse::getInFaces(size_t k) const
{
    if (k >= inFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)inFaces_arraysize, (unsigned long)k);
    return this->inFaces[k];
}

void PITLookupResponse::setInFacesArraySize(size_t newSize)
{
    int *inFaces2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = inFaces_arraysize < newSize ? inFaces_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        inFaces2[i] = this->inFaces[i];
    for (size_t i = minSize; i < newSize; i++)
        inFaces2[i] = 0;
    delete [] this->inFaces;
    this->inFaces = inFaces2;
    inFaces_arraysize = newSize;
}

void PITLookupResponse::setInFaces(size_t k, int inFaces)
{
    if (k >= inFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)inFaces_arraysize, (unsigned long)k);
    this->inFaces[k] = inFaces;
}

void PITLookupResponse::insertInFaces(size_t k, int inFaces)
{
    if (k > inFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)inFaces_arraysize, (unsigned long)k);
    size_t newSize = inFaces_arraysize + 1;
    int *inFaces2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        inFaces2[i] = this->inFaces[i];
    inFaces2[k] = inFaces;
    for (i = k + 1; i < newSize; i++)
        inFaces2[i] = this->inFaces[i-1];
    delete [] this->inFaces;
    this->inFaces = inFaces2;
    inFaces_arraysize = newSize;
}

void PITLookupResponse::appendInFaces(int inFaces)
{
    insertInFaces(inFaces_arraysize, inFaces);
}

void PITLookupResponse::eraseInFaces(size_t k)
{
    if (k >= inFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)inFaces_arraysize, (unsigned long)k);
    size_t newSize = inFaces_arraysize - 1;
    int *inFaces2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        inFaces2[i] = this->inFaces[i];
    for (i = k; i < newSize; i++)
        inFaces2[i] = this->inFaces[i+1];
    delete [] this->inFaces;
    this->inFaces = inFaces2;
    inFaces_arraysize = newSize;
}

size_t PITLookupResponse::getOutFacesArraySize() const
{
    return outFaces_arraysize;
}

int PITLookupResponse::getOutFaces(size_t k) const
{
    if (k >= outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    return this->outFaces[k];
}

void PITLookupResponse::setOutFacesArraySize(size_t newSize)
{
    int *outFaces2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = outFaces_arraysize < newSize ? outFaces_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        outFaces2[i] = this->outFaces[i];
    for (size_t i = minSize; i < newSize; i++)
        outFaces2[i] = 0;
    delete [] this->outFaces;
    this->outFaces = outFaces2;
    outFaces_arraysize = newSize;
}

void PITLookupResponse::setOutFaces(size_t k, int outFaces)
{
    if (k >= outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    this->outFaces[k] = outFaces;
}

void PITLookupResponse::insertOutFaces(size_t k, int outFaces)
{
    if (k > outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    size_t newSize = outFaces_arraysize + 1;
    int *outFaces2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        outFaces2[i] = this->outFaces[i];
    outFaces2[k] = outFaces;
    for (i = k + 1; i < newSize; i++)
        outFaces2[i] = this->outFaces[i-1];
    delete [] this->outFaces;
    this->outFaces = outFaces2;
    outFaces_arraysize = newSize;
}

void PITLookupResponse::appendOutFaces(int outFaces)
{
    insertOutFaces(outFaces_arraysize, outFaces);
}

void PITLookupResponse::eraseOutFaces(size_t k)
{
    if (k >= outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    size_t newSize = outFaces_arraysize - 1;
    int *outFaces2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        outFaces2[i] = this->outFaces[i];
    for (i = k; i < newSize; i++)
        outFaces2[i] = this->outFaces[i+1];
    delete [] this->outFaces;
    this->outFaces = outFaces2;
    outFaces_arraysize = newSize;
}

class PITLookupResponseDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_found,
        FIELD_inFaces,
        FIELD_outFaces,
    };
  public:
    PITLookupResponseDescriptor();
    virtual ~PITLookupResponseDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(PITLookupResponseDescriptor)

PITLookupResponseDescriptor::PITLookupResponseDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::PITLookupResponse)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

PITLookupResponseDescriptor::~PITLookupResponseDescriptor()
{
    delete[] propertyNames;
}

bool PITLookupResponseDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<PITLookupResponse *>(obj)!=nullptr;
}

const char **PITLookupResponseDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *PITLookupResponseDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string PITLookupResponseDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void PITLookupResponseDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int PITLookupResponseDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int PITLookupResponseDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_found
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_inFaces
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_outFaces
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *PITLookupResponseDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "found",
        "inFaces",
        "outFaces",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int PITLookupResponseDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "found") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "inFaces") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "outFaces") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *PITLookupResponseDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_found
        "int",    // FIELD_inFaces
        "int",    // FIELD_outFaces
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **PITLookupResponseDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *PITLookupResponseDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int PITLookupResponseDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_inFaces: return pp->getInFacesArraySize();
        case FIELD_outFaces: return pp->getOutFacesArraySize();
        default: return 0;
    }
}

void PITLookupResponseDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_inFaces: pp->setInFacesArraySize(size); break;
        case FIELD_outFaces: pp->setOutFacesArraySize(size); break;
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'PITLookupResponse'", field);
    }
}

const char *PITLookupResponseDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string PITLookupResponseDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: return bool2string(pp->getFound());
        case FIELD_inFaces: return long2string(pp->getInFaces(i));
        case FIELD_outFaces: return long2string(pp->getOutFaces(i));
        default: return "";
    }
}

void PITLookupResponseDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: pp->setFound(string2bool(value)); break;
        case FIELD_inFaces: pp->setInFaces(i,string2long(value)); break;
        case FIELD_outFaces: pp->setOutFaces(i,string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITLookupResponse'", field);
    }
}

omnetpp::cValue PITLookupResponseDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: return pp->getFound();
        case FIELD_inFaces: return pp->getInFaces(i);
        case FIELD_outFaces: return pp->getOutFaces(i);
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'PITLookupResponse' as cValue -- field index out of range?", field);
    }
}

void PITLookupResponseDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: pp->setFound(value.boolValue()); break;
        case FIELD_inFaces: pp->setInFaces(i,omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_outFaces: pp->setOutFaces(i,omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITLookupResponse'", field);
    }
}

const char *PITLookupResponseDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr PITLookupResponseDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void PITLookupResponseDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    PITLookupResponse *pp = omnetpp::fromAnyPtr<PITLookupResponse>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITLookupResponse'", field);
    }
}

Register_Class(PITSatisfyRequest)

PITSatisfyRequest::PITSatisfyRequest(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(PIT_SATISFY_REQUEST);

}

PITSatisfyRequest::PITSatisfyRequest(const PITSatisfyRequest& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

PITSatisfyRequest::~PITSatisfyRequest()
{
}

PITSatisfyRequest& PITSatisfyRequest::operator=(const PITSatisfyRequest& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void PITSatisfyRequest::copy(const PITSatisfyRequest& other)
{
    this->inFace = other.inFace;
}

void PITSatisfyRequest::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->inFace);
}

void PITSatisfyRequest::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->inFace);
}

int PITSatisfyRequest::getInFace() const
{
    return this->inFace;
}

void PITSatisfyRequest::setInFace(int inFace)
{
    this->inFace = inFace;
}

class PITSatisfyRequestDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_inFace,
    };
  public:
    PITSatisfyRequestDescriptor();
    virtual ~PITSatisfyRequestDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(PITSatisfyRequestDescriptor)

PITSatisfyRequestDescriptor::PITSatisfyRequestDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::PITSatisfyRequest)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

PITSatisfyRequestDescriptor::~PITSatisfyRequestDescriptor()
{
    delete[] propertyNames;
}

bool PITSatisfyRequestDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<PITSatisfyRequest *>(obj)!=nullptr;
}

const char **PITSatisfyRequestDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *PITSatisfyRequestDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string PITSatisfyRequestDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void PITSatisfyRequestDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int PITSatisfyRequestDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 1+base->getFieldCount() : 1;
}

unsigned int PITSatisfyRequestDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_inFace
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *PITSatisfyRequestDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "inFace",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int PITSatisfyRequestDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "inFace") == 0) return baseIndex + 0;
    return base ? base->findField(fieldName) : -1;
}

const char *PITSatisfyRequestDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_inFace
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **PITSatisfyRequestDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *PITSatisfyRequestDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int PITSatisfyRequestDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void PITSatisfyRequestDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'PITSatisfyRequest'", field);
    }
}

const char *PITSatisfyRequestDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string PITSatisfyRequestDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: return long2string(pp->getInFace());
        default: return "";
    }
}

void PITSatisfyRequestDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: pp->setInFace(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITSatisfyRequest'", field);
    }
}

omnetpp::cValue PITSatisfyRequestDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: return pp->getInFace();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'PITSatisfyRequest' as cValue -- field index out of range?", field);
    }
}

void PITSatisfyRequestDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: pp->setInFace(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITSatisfyRequest'", field);
    }
}

const char *PITSatisfyRequestDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr PITSatisfyRequestDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void PITSatisfyRequestDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    PITSatisfyRequest *pp = omnetpp::fromAnyPtr<PITSatisfyRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITSatisfyRequest'", field);
    }
}

Register_Class(PITSatisfyResponse)

PITSatisfyResponse::PITSatisfyResponse(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(PIT_SATISFY_RESPONSE);

}

PITSatisfyResponse::PITSatisfyResponse(const PITSatisfyResponse& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

PITSatisfyResponse::~PITSatisfyResponse()
{
    delete [] this->outFaces;
}

PITSatisfyResponse& PITSatisfyResponse::operator=(const PITSatisfyResponse& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void PITSatisfyResponse::copy(const PITSatisfyResponse& other)
{
    this->found = other.found;
    delete [] this->outFaces;
    this->outFaces = (other.outFaces_arraysize==0) ? nullptr : new int[other.outFaces_arraysize];
    outFaces_arraysize = other.outFaces_arraysize;
    for (size_t i = 0; i < outFaces_arraysize; i++) {
        this->outFaces[i] = other.outFaces[i];
    }
}

void PITSatisfyResponse::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->found);
    b->pack(outFaces_arraysize);
    doParsimArrayPacking(b,this->outFaces,outFaces_arraysize);
}

void PITSatisfyResponse::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->found);
    delete [] this->outFaces;
    b->unpack(outFaces_arraysize);
    if (outFaces_arraysize == 0) {
        this->outFaces = nullptr;
    } else {
        this->outFaces = new int[outFaces_arraysize];
        doParsimArrayUnpacking(b,this->outFaces,outFaces_arraysize);
    }
}

bool PITSatisfyResponse::getFound() const
{
    return this->found;
}

void PITSatisfyResponse::setFound(bool found)
{
    this->found = found;
}

size_t PITSatisfyResponse::getOutFacesArraySize() const
{
    return outFaces_arraysize;
}

int PITSatisfyResponse::getOutFaces(size_t k) const
{
    if (k >= outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    return this->outFaces[k];
}

void PITSatisfyResponse::setOutFacesArraySize(size_t newSize)
{
    int *outFaces2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = outFaces_arraysize < newSize ? outFaces_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        outFaces2[i] = this->outFaces[i];
    for (size_t i = minSize; i < newSize; i++)
        outFaces2[i] = 0;
    delete [] this->outFaces;
    this->outFaces = outFaces2;
    outFaces_arraysize = newSize;
}

void PITSatisfyResponse::setOutFaces(size_t k, int outFaces)
{
    if (k >= outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    this->outFaces[k] = outFaces;
}

void PITSatisfyResponse::insertOutFaces(size_t k, int outFaces)
{
    if (k > outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    size_t newSize = outFaces_arraysize + 1;
    int *outFaces2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        outFaces2[i] = this->outFaces[i];
    outFaces2[k] = outFaces;
    for (i = k + 1; i < newSize; i++)
        outFaces2[i] = this->outFaces[i-1];
    delete [] this->outFaces;
    this->outFaces = outFaces2;
    outFaces_arraysize = newSize;
}

void PITSatisfyResponse::appendOutFaces(int outFaces)
{
    insertOutFaces(outFaces_arraysize, outFaces);
}

void PITSatisfyResponse::eraseOutFaces(size_t k)
{
    if (k >= outFaces_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)outFaces_arraysize, (unsigned long)k);
    size_t newSize = outFaces_arraysize - 1;
    int *outFaces2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        outFaces2[i] = this->outFaces[i];
    for (i = k; i < newSize; i++)
        outFaces2[i] = this->outFaces[i+1];
    delete [] this->outFaces;
    this->outFaces = outFaces2;
    outFaces_arraysize = newSize;
}

class PITSatisfyResponseDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_found,
        FIELD_outFaces,
    };
  public:
    PITSatisfyResponseDescriptor();
    virtual ~PITSatisfyResponseDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(PITSatisfyResponseDescriptor)

PITSatisfyResponseDescriptor::PITSatisfyResponseDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::PITSatisfyResponse)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

PITSatisfyResponseDescriptor::~PITSatisfyResponseDescriptor()
{
    delete[] propertyNames;
}

bool PITSatisfyResponseDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<PITSatisfyResponse *>(obj)!=nullptr;
}

const char **PITSatisfyResponseDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *PITSatisfyResponseDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string PITSatisfyResponseDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void PITSatisfyResponseDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int PITSatisfyResponseDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 2+base->getFieldCount() : 2;
}

unsigned int PITSatisfyResponseDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_found
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_outFaces
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *PITSatisfyResponseDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "found",
        "outFaces",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int PITSatisfyResponseDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "found") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "outFaces") == 0) return baseIndex + 1;
    return base ? base->findField(fieldName) : -1;
}

const char *PITSatisfyResponseDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_found
        "int",    // FIELD_outFaces
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **PITSatisfyResponseDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *PITSatisfyResponseDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int PITSatisfyResponseDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    switch (field) {
        case FIELD_outFaces: return pp->getOutFacesArraySize();
        default: return 0;
    }
}

void PITSatisfyResponseDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    switch (field) {
        case FIELD_outFaces: pp->setOutFacesArraySize(size); break;
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'PITSatisfyResponse'", field);
    }
}

const char *PITSatisfyResponseDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string PITSatisfyResponseDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: return bool2string(pp->getFound());
        case FIELD_outFaces: return long2string(pp->getOutFaces(i));
        default: return "";
    }
}

void PITSatisfyResponseDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: pp->setFound(string2bool(value)); break;
        case FIELD_outFaces: pp->setOutFaces(i,string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITSatisfyResponse'", field);
    }
}

omnetpp::cValue PITSatisfyResponseDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: return pp->getFound();
        case FIELD_outFaces: return pp->getOutFaces(i);
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'PITSatisfyResponse' as cValue -- field index out of range?", field);
    }
}

void PITSatisfyResponseDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: pp->setFound(value.boolValue()); break;
        case FIELD_outFaces: pp->setOutFaces(i,omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITSatisfyResponse'", field);
    }
}

const char *PITSatisfyResponseDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr PITSatisfyResponseDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void PITSatisfyResponseDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    PITSatisfyResponse *pp = omnetpp::fromAnyPtr<PITSatisfyResponse>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'PITSatisfyResponse'", field);
    }
}

Register_Class(FIBLookupRequest)

FIBLookupRequest::FIBLookupRequest(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(FIB_LOOKUP_REQUEST);

}

FIBLookupRequest::FIBLookupRequest(const FIBLookupRequest& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

FIBLookupRequest::~FIBLookupRequest()
{
}

FIBLookupRequest& FIBLookupRequest::operator=(const FIBLookupRequest& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void FIBLookupRequest::copy(const FIBLookupRequest& other)
{
    this->inFace = other.inFace;
}

void FIBLookupRequest::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->inFace);
}

void FIBLookupRequest::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->inFace);
}

int FIBLookupRequest::getInFace() const
{
    return this->inFace;
}

void FIBLookupRequest::setInFace(int inFace)
{
    this->inFace = inFace;
}

class FIBLookupRequestDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_inFace,
    };
  public:
    FIBLookupRequestDescriptor();
    virtual ~FIBLookupRequestDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(FIBLookupRequestDescriptor)

FIBLookupRequestDescriptor::FIBLookupRequestDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::FIBLookupRequest)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

FIBLookupRequestDescriptor::~FIBLookupRequestDescriptor()
{
    delete[] propertyNames;
}

bool FIBLookupRequestDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<FIBLookupRequest *>(obj)!=nullptr;
}

const char **FIBLookupRequestDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *FIBLookupRequestDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string FIBLookupRequestDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void FIBLookupRequestDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int FIBLookupRequestDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 1+base->getFieldCount() : 1;
}

unsigned int FIBLookupRequestDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_inFace
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *FIBLookupRequestDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "inFace",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int FIBLookupRequestDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "inFace") == 0) return baseIndex + 0;
    return base ? base->findField(fieldName) : -1;
}

const char *FIBLookupRequestDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_inFace
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **FIBLookupRequestDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *FIBLookupRequestDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int FIBLookupRequestDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void FIBLookupRequestDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'FIBLookupRequest'", field);
    }
}

const char *FIBLookupRequestDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string FIBLookupRequestDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: return long2string(pp->getInFace());
        default: return "";
    }
}

void FIBLookupRequestDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: pp->setInFace(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBLookupRequest'", field);
    }
}

omnetpp::cValue FIBLookupRequestDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: return pp->getInFace();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'FIBLookupRequest' as cValue -- field index out of range?", field);
    }
}

void FIBLookupRequestDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    switch (field) {
        case FIELD_inFace: pp->setInFace(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBLookupRequest'", field);
    }
}

const char *FIBLookupRequestDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr FIBLookupRequestDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void FIBLookupRequestDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBLookupRequest *pp = omnetpp::fromAnyPtr<FIBLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBLookupRequest'", field);
    }
}

Register_Class(FIBLookupResponse)

FIBLookupResponse::FIBLookupResponse(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(FIB_LOOKUP_RESPONSE);

}

FIBLookupResponse::FIBLookupResponse(const FIBLookupResponse& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

FIBLookupResponse::~FIBLookupResponse()
{
    delete [] this->nextHops;
    delete [] this->costs;
}

FIBLookupResponse& FIBLookupResponse::operator=(const FIBLookupResponse& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void FIBLookupResponse::copy(const FIBLookupResponse& other)
{
    this->found = other.found;
    delete [] this->nextHops;
    this->nextHops = (other.nextHops_arraysize==0) ? nullptr : new int[other.nextHops_arraysize];
    nextHops_arraysize = other.nextHops_arraysize;
    for (size_t i = 0; i < nextHops_arraysize; i++) {
        this->nextHops[i] = other.nextHops[i];
    }
    delete [] this->costs;
    this->costs = (other.costs_arraysize==0) ? nullptr : new int[other.costs_arraysize];
    costs_arraysize = other.costs_arraysize;
    for (size_t i = 0; i < costs_arraysize; i++) {
        this->costs[i] = other.costs[i];
    }
}

void FIBLookupResponse::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->found);
    b->pack(nextHops_arraysize);
    doParsimArrayPacking(b,this->nextHops,nextHops_arraysize);
    b->pack(costs_arraysize);
    doParsimArrayPacking(b,this->costs,costs_arraysize);
}

void FIBLookupResponse::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->found);
    delete [] this->nextHops;
    b->unpack(nextHops_arraysize);
    if (nextHops_arraysize == 0) {
        this->nextHops = nullptr;
    } else {
        this->nextHops = new int[nextHops_arraysize];
        doParsimArrayUnpacking(b,this->nextHops,nextHops_arraysize);
    }
    delete [] this->costs;
    b->unpack(costs_arraysize);
    if (costs_arraysize == 0) {
        this->costs = nullptr;
    } else {
        this->costs = new int[costs_arraysize];
        doParsimArrayUnpacking(b,this->costs,costs_arraysize);
    }
}

bool FIBLookupResponse::getFound() const
{
    return this->found;
}

void FIBLookupResponse::setFound(bool found)
{
    this->found = found;
}

size_t FIBLookupResponse::getNextHopsArraySize() const
{
    return nextHops_arraysize;
}

int FIBLookupResponse::getNextHops(size_t k) const
{
    if (k >= nextHops_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)nextHops_arraysize, (unsigned long)k);
    return this->nextHops[k];
}

void FIBLookupResponse::setNextHopsArraySize(size_t newSize)
{
    int *nextHops2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = nextHops_arraysize < newSize ? nextHops_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        nextHops2[i] = this->nextHops[i];
    for (size_t i = minSize; i < newSize; i++)
        nextHops2[i] = 0;
    delete [] this->nextHops;
    this->nextHops = nextHops2;
    nextHops_arraysize = newSize;
}

void FIBLookupResponse::setNextHops(size_t k, int nextHops)
{
    if (k >= nextHops_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)nextHops_arraysize, (unsigned long)k);
    this->nextHops[k] = nextHops;
}

void FIBLookupResponse::insertNextHops(size_t k, int nextHops)
{
    if (k > nextHops_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)nextHops_arraysize, (unsigned long)k);
    size_t newSize = nextHops_arraysize + 1;
    int *nextHops2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        nextHops2[i] = this->nextHops[i];
    nextHops2[k] = nextHops;
    for (i = k + 1; i < newSize; i++)
        nextHops2[i] = this->nextHops[i-1];
    delete [] this->nextHops;
    this->nextHops = nextHops2;
    nextHops_arraysize = newSize;
}

void FIBLookupResponse::appendNextHops(int nextHops)
{
    insertNextHops(nextHops_arraysize, nextHops);
}

void FIBLookupResponse::eraseNextHops(size_t k)
{
    if (k >= nextHops_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)nextHops_arraysize, (unsigned long)k);
    size_t newSize = nextHops_arraysize - 1;
    int *nextHops2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        nextHops2[i] = this->nextHops[i];
    for (i = k; i < newSize; i++)
        nextHops2[i] = this->nextHops[i+1];
    delete [] this->nextHops;
    this->nextHops = nextHops2;
    nextHops_arraysize = newSize;
}

size_t FIBLookupResponse::getCostsArraySize() const
{
    return costs_arraysize;
}

int FIBLookupResponse::getCosts(size_t k) const
{
    if (k >= costs_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)costs_arraysize, (unsigned long)k);
    return this->costs[k];
}

void FIBLookupResponse::setCostsArraySize(size_t newSize)
{
    int *costs2 = (newSize==0) ? nullptr : new int[newSize];
    size_t minSize = costs_arraysize < newSize ? costs_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        costs2[i] = this->costs[i];
    for (size_t i = minSize; i < newSize; i++)
        costs2[i] = 0;
    delete [] this->costs;
    this->costs = costs2;
    costs_arraysize = newSize;
}

void FIBLookupResponse::setCosts(size_t k, int costs)
{
    if (k >= costs_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)costs_arraysize, (unsigned long)k);
    this->costs[k] = costs;
}

void FIBLookupResponse::insertCosts(size_t k, int costs)
{
    if (k > costs_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)costs_arraysize, (unsigned long)k);
    size_t newSize = costs_arraysize + 1;
    int *costs2 = new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        costs2[i] = this->costs[i];
    costs2[k] = costs;
    for (i = k + 1; i < newSize; i++)
        costs2[i] = this->costs[i-1];
    delete [] this->costs;
    this->costs = costs2;
    costs_arraysize = newSize;
}

void FIBLookupResponse::appendCosts(int costs)
{
    insertCosts(costs_arraysize, costs);
}

void FIBLookupResponse::eraseCosts(size_t k)
{
    if (k >= costs_arraysize) throw omnetpp::cRuntimeError("Array of size %lu indexed by %lu", (unsigned long)costs_arraysize, (unsigned long)k);
    size_t newSize = costs_arraysize - 1;
    int *costs2 = (newSize == 0) ? nullptr : new int[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        costs2[i] = this->costs[i];
    for (i = k; i < newSize; i++)
        costs2[i] = this->costs[i+1];
    delete [] this->costs;
    this->costs = costs2;
    costs_arraysize = newSize;
}

class FIBLookupResponseDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_found,
        FIELD_nextHops,
        FIELD_costs,
    };
  public:
    FIBLookupResponseDescriptor();
    virtual ~FIBLookupResponseDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(FIBLookupResponseDescriptor)

FIBLookupResponseDescriptor::FIBLookupResponseDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::FIBLookupResponse)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

FIBLookupResponseDescriptor::~FIBLookupResponseDescriptor()
{
    delete[] propertyNames;
}

bool FIBLookupResponseDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<FIBLookupResponse *>(obj)!=nullptr;
}

const char **FIBLookupResponseDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *FIBLookupResponseDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string FIBLookupResponseDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void FIBLookupResponseDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int FIBLookupResponseDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int FIBLookupResponseDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_found
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_nextHops
        FD_ISARRAY | FD_ISEDITABLE | FD_ISRESIZABLE,    // FIELD_costs
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *FIBLookupResponseDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "found",
        "nextHops",
        "costs",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int FIBLookupResponseDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "found") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "nextHops") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "costs") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *FIBLookupResponseDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_found
        "int",    // FIELD_nextHops
        "int",    // FIELD_costs
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **FIBLookupResponseDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *FIBLookupResponseDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int FIBLookupResponseDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_nextHops: return pp->getNextHopsArraySize();
        case FIELD_costs: return pp->getCostsArraySize();
        default: return 0;
    }
}

void FIBLookupResponseDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_nextHops: pp->setNextHopsArraySize(size); break;
        case FIELD_costs: pp->setCostsArraySize(size); break;
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'FIBLookupResponse'", field);
    }
}

const char *FIBLookupResponseDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string FIBLookupResponseDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: return bool2string(pp->getFound());
        case FIELD_nextHops: return long2string(pp->getNextHops(i));
        case FIELD_costs: return long2string(pp->getCosts(i));
        default: return "";
    }
}

void FIBLookupResponseDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: pp->setFound(string2bool(value)); break;
        case FIELD_nextHops: pp->setNextHops(i,string2long(value)); break;
        case FIELD_costs: pp->setCosts(i,string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBLookupResponse'", field);
    }
}

omnetpp::cValue FIBLookupResponseDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: return pp->getFound();
        case FIELD_nextHops: return pp->getNextHops(i);
        case FIELD_costs: return pp->getCosts(i);
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'FIBLookupResponse' as cValue -- field index out of range?", field);
    }
}

void FIBLookupResponseDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: pp->setFound(value.boolValue()); break;
        case FIELD_nextHops: pp->setNextHops(i,omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_costs: pp->setCosts(i,omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBLookupResponse'", field);
    }
}

const char *FIBLookupResponseDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr FIBLookupResponseDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void FIBLookupResponseDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBLookupResponse *pp = omnetpp::fromAnyPtr<FIBLookupResponse>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBLookupResponse'", field);
    }
}

Register_Class(FIBAddRouteRequest)

FIBAddRouteRequest::FIBAddRouteRequest(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(FIB_ADD_ROUTE_REQUEST);

}

FIBAddRouteRequest::FIBAddRouteRequest(const FIBAddRouteRequest& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

FIBAddRouteRequest::~FIBAddRouteRequest()
{
}

FIBAddRouteRequest& FIBAddRouteRequest::operator=(const FIBAddRouteRequest& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void FIBAddRouteRequest::copy(const FIBAddRouteRequest& other)
{
    this->prefix = other.prefix;
    this->nextHop = other.nextHop;
    this->cost = other.cost;
}

void FIBAddRouteRequest::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->prefix);
    doParsimPacking(b,this->nextHop);
    doParsimPacking(b,this->cost);
}

void FIBAddRouteRequest::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->prefix);
    doParsimUnpacking(b,this->nextHop);
    doParsimUnpacking(b,this->cost);
}

const char * FIBAddRouteRequest::getPrefix() const
{
    return this->prefix.c_str();
}

void FIBAddRouteRequest::setPrefix(const char * prefix)
{
    this->prefix = prefix;
}

int FIBAddRouteRequest::getNextHop() const
{
    return this->nextHop;
}

void FIBAddRouteRequest::setNextHop(int nextHop)
{
    this->nextHop = nextHop;
}

int FIBAddRouteRequest::getCost() const
{
    return this->cost;
}

void FIBAddRouteRequest::setCost(int cost)
{
    this->cost = cost;
}

class FIBAddRouteRequestDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_prefix,
        FIELD_nextHop,
        FIELD_cost,
    };
  public:
    FIBAddRouteRequestDescriptor();
    virtual ~FIBAddRouteRequestDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(FIBAddRouteRequestDescriptor)

FIBAddRouteRequestDescriptor::FIBAddRouteRequestDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::FIBAddRouteRequest)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

FIBAddRouteRequestDescriptor::~FIBAddRouteRequestDescriptor()
{
    delete[] propertyNames;
}

bool FIBAddRouteRequestDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<FIBAddRouteRequest *>(obj)!=nullptr;
}

const char **FIBAddRouteRequestDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *FIBAddRouteRequestDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string FIBAddRouteRequestDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void FIBAddRouteRequestDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int FIBAddRouteRequestDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int FIBAddRouteRequestDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_prefix
        FD_ISEDITABLE,    // FIELD_nextHop
        FD_ISEDITABLE,    // FIELD_cost
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *FIBAddRouteRequestDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "prefix",
        "nextHop",
        "cost",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int FIBAddRouteRequestDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "prefix") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "nextHop") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "cost") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *FIBAddRouteRequestDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "string",    // FIELD_prefix
        "int",    // FIELD_nextHop
        "int",    // FIELD_cost
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **FIBAddRouteRequestDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *FIBAddRouteRequestDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int FIBAddRouteRequestDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void FIBAddRouteRequestDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'FIBAddRouteRequest'", field);
    }
}

const char *FIBAddRouteRequestDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string FIBAddRouteRequestDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    switch (field) {
        case FIELD_prefix: return oppstring2string(pp->getPrefix());
        case FIELD_nextHop: return long2string(pp->getNextHop());
        case FIELD_cost: return long2string(pp->getCost());
        default: return "";
    }
}

void FIBAddRouteRequestDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    switch (field) {
        case FIELD_prefix: pp->setPrefix((value)); break;
        case FIELD_nextHop: pp->setNextHop(string2long(value)); break;
        case FIELD_cost: pp->setCost(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBAddRouteRequest'", field);
    }
}

omnetpp::cValue FIBAddRouteRequestDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    switch (field) {
        case FIELD_prefix: return pp->getPrefix();
        case FIELD_nextHop: return pp->getNextHop();
        case FIELD_cost: return pp->getCost();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'FIBAddRouteRequest' as cValue -- field index out of range?", field);
    }
}

void FIBAddRouteRequestDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    switch (field) {
        case FIELD_prefix: pp->setPrefix(value.stringValue()); break;
        case FIELD_nextHop: pp->setNextHop(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_cost: pp->setCost(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBAddRouteRequest'", field);
    }
}

const char *FIBAddRouteRequestDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr FIBAddRouteRequestDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void FIBAddRouteRequestDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBAddRouteRequest *pp = omnetpp::fromAnyPtr<FIBAddRouteRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBAddRouteRequest'", field);
    }
}

Register_Class(FIBAddRouteResponse)

FIBAddRouteResponse::FIBAddRouteResponse(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(FIB_ADD_ROUTE_RESPONSE);

}

FIBAddRouteResponse::FIBAddRouteResponse(const FIBAddRouteResponse& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

FIBAddRouteResponse::~FIBAddRouteResponse()
{
}

FIBAddRouteResponse& FIBAddRouteResponse::operator=(const FIBAddRouteResponse& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void FIBAddRouteResponse::copy(const FIBAddRouteResponse& other)
{
    this->success = other.success;
}

void FIBAddRouteResponse::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->success);
}

void FIBAddRouteResponse::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->success);
}

bool FIBAddRouteResponse::getSuccess() const
{
    return this->success;
}

void FIBAddRouteResponse::setSuccess(bool success)
{
    this->success = success;
}

class FIBAddRouteResponseDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_success,
    };
  public:
    FIBAddRouteResponseDescriptor();
    virtual ~FIBAddRouteResponseDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(FIBAddRouteResponseDescriptor)

FIBAddRouteResponseDescriptor::FIBAddRouteResponseDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::FIBAddRouteResponse)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

FIBAddRouteResponseDescriptor::~FIBAddRouteResponseDescriptor()
{
    delete[] propertyNames;
}

bool FIBAddRouteResponseDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<FIBAddRouteResponse *>(obj)!=nullptr;
}

const char **FIBAddRouteResponseDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *FIBAddRouteResponseDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string FIBAddRouteResponseDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void FIBAddRouteResponseDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int FIBAddRouteResponseDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 1+base->getFieldCount() : 1;
}

unsigned int FIBAddRouteResponseDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_success
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *FIBAddRouteResponseDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "success",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int FIBAddRouteResponseDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "success") == 0) return baseIndex + 0;
    return base ? base->findField(fieldName) : -1;
}

const char *FIBAddRouteResponseDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_success
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **FIBAddRouteResponseDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *FIBAddRouteResponseDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int FIBAddRouteResponseDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void FIBAddRouteResponseDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'FIBAddRouteResponse'", field);
    }
}

const char *FIBAddRouteResponseDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string FIBAddRouteResponseDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: return bool2string(pp->getSuccess());
        default: return "";
    }
}

void FIBAddRouteResponseDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: pp->setSuccess(string2bool(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBAddRouteResponse'", field);
    }
}

omnetpp::cValue FIBAddRouteResponseDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: return pp->getSuccess();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'FIBAddRouteResponse' as cValue -- field index out of range?", field);
    }
}

void FIBAddRouteResponseDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: pp->setSuccess(value.boolValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBAddRouteResponse'", field);
    }
}

const char *FIBAddRouteResponseDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr FIBAddRouteResponseDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void FIBAddRouteResponseDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    FIBAddRouteResponse *pp = omnetpp::fromAnyPtr<FIBAddRouteResponse>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'FIBAddRouteResponse'", field);
    }
}

Register_Class(CSLookupRequest)

CSLookupRequest::CSLookupRequest(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(CS_LOOKUP_REQUEST);
}

CSLookupRequest::CSLookupRequest(const CSLookupRequest& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

CSLookupRequest::~CSLookupRequest()
{
}

CSLookupRequest& CSLookupRequest::operator=(const CSLookupRequest& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void CSLookupRequest::copy(const CSLookupRequest& other)
{
}

void CSLookupRequest::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
}

void CSLookupRequest::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
}

class CSLookupRequestDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
    };
  public:
    CSLookupRequestDescriptor();
    virtual ~CSLookupRequestDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(CSLookupRequestDescriptor)

CSLookupRequestDescriptor::CSLookupRequestDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::CSLookupRequest)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

CSLookupRequestDescriptor::~CSLookupRequestDescriptor()
{
    delete[] propertyNames;
}

bool CSLookupRequestDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<CSLookupRequest *>(obj)!=nullptr;
}

const char **CSLookupRequestDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *CSLookupRequestDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string CSLookupRequestDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void CSLookupRequestDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int CSLookupRequestDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 0+base->getFieldCount() : 0;
}

unsigned int CSLookupRequestDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    return 0;
}

const char *CSLookupRequestDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

int CSLookupRequestDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->findField(fieldName) : -1;
}

const char *CSLookupRequestDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

const char **CSLookupRequestDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *CSLookupRequestDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int CSLookupRequestDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void CSLookupRequestDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'CSLookupRequest'", field);
    }
}

const char *CSLookupRequestDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string CSLookupRequestDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    switch (field) {
        default: return "";
    }
}

void CSLookupRequestDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSLookupRequest'", field);
    }
}

omnetpp::cValue CSLookupRequestDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'CSLookupRequest' as cValue -- field index out of range?", field);
    }
}

void CSLookupRequestDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSLookupRequest'", field);
    }
}

const char *CSLookupRequestDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    return nullptr;
}

omnetpp::any_ptr CSLookupRequestDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void CSLookupRequestDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    CSLookupRequest *pp = omnetpp::fromAnyPtr<CSLookupRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSLookupRequest'", field);
    }
}

Register_Class(CSLookupResponse)

CSLookupResponse::CSLookupResponse(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(CS_LOOKUP_RESPONSE);

    if (this->data != nullptr) take(this->data);
}

CSLookupResponse::CSLookupResponse(const CSLookupResponse& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

CSLookupResponse::~CSLookupResponse()
{
    dropAndDelete(this->data);
}

CSLookupResponse& CSLookupResponse::operator=(const CSLookupResponse& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void CSLookupResponse::copy(const CSLookupResponse& other)
{
    this->found = other.found;
    dropAndDelete(this->data);
    this->data = other.data;
    if (this->data != nullptr) {
        this->data = this->data->dup();
        take(this->data);
        this->data->setName(other.data->getName());
    }
}

void CSLookupResponse::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->found);
    doParsimPacking(b,this->data);
}

void CSLookupResponse::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->found);
    doParsimUnpacking(b,this->data);
}

bool CSLookupResponse::getFound() const
{
    return this->found;
}

void CSLookupResponse::setFound(bool found)
{
    this->found = found;
}

const ::omnetpp::cMessage * CSLookupResponse::getData() const
{
    return this->data;
}

void CSLookupResponse::setData(::omnetpp::cMessage * data)
{
    if (data == this->data) return;
    if (this->data != nullptr) dropAndDelete(this->data);
    this->data = data;
    if (this->data != nullptr) take(this->data);
}

::omnetpp::cMessage * CSLookupResponse::removeData()
{
    ::omnetpp::cMessage * retval = this->data;
    if (retval != nullptr) drop(retval);
    this->data = nullptr;
    return retval;
}

class CSLookupResponseDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_found,
        FIELD_data,
    };
  public:
    CSLookupResponseDescriptor();
    virtual ~CSLookupResponseDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(CSLookupResponseDescriptor)

CSLookupResponseDescriptor::CSLookupResponseDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::CSLookupResponse)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

CSLookupResponseDescriptor::~CSLookupResponseDescriptor()
{
    delete[] propertyNames;
}

bool CSLookupResponseDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<CSLookupResponse *>(obj)!=nullptr;
}

const char **CSLookupResponseDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *CSLookupResponseDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string CSLookupResponseDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void CSLookupResponseDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int CSLookupResponseDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 2+base->getFieldCount() : 2;
}

unsigned int CSLookupResponseDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_found
        FD_ISCOMPOUND | FD_ISPOINTER | FD_ISCOBJECT | FD_ISCOWNEDOBJECT | FD_ISREPLACEABLE,    // FIELD_data
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *CSLookupResponseDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "found",
        "data",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int CSLookupResponseDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "found") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "data") == 0) return baseIndex + 1;
    return base ? base->findField(fieldName) : -1;
}

const char *CSLookupResponseDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_found
        "omnetpp::cMessage",    // FIELD_data
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **CSLookupResponseDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_data: {
            static const char *names[] = { "owned", "allowReplace",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *CSLookupResponseDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_data:
            if (!strcmp(propertyName, "owned")) return "";
            if (!strcmp(propertyName, "allowReplace")) return "";
            return nullptr;
        default: return nullptr;
    }
}

int CSLookupResponseDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void CSLookupResponseDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'CSLookupResponse'", field);
    }
}

const char *CSLookupResponseDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_data: { const ::omnetpp::cMessage * value = pp->getData(); return omnetpp::opp_typename(typeid(*value)); }
        default: return nullptr;
    }
}

std::string CSLookupResponseDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: return bool2string(pp->getFound());
        case FIELD_data: { auto obj = pp->getData(); return obj == nullptr ? "" : obj->str(); }
        default: return "";
    }
}

void CSLookupResponseDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: pp->setFound(string2bool(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSLookupResponse'", field);
    }
}

omnetpp::cValue CSLookupResponseDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: return pp->getFound();
        case FIELD_data: return omnetpp::toAnyPtr(pp->getData()); break;
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'CSLookupResponse' as cValue -- field index out of range?", field);
    }
}

void CSLookupResponseDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_found: pp->setFound(value.boolValue()); break;
        case FIELD_data: pp->setData(omnetpp::fromAnyPtr<::omnetpp::cMessage>(value.pointerValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSLookupResponse'", field);
    }
}

const char *CSLookupResponseDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_data: return omnetpp::opp_typename(typeid(::omnetpp::cMessage));
        default: return nullptr;
    };
}

omnetpp::any_ptr CSLookupResponseDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_data: return omnetpp::toAnyPtr(pp->getData()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void CSLookupResponseDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    CSLookupResponse *pp = omnetpp::fromAnyPtr<CSLookupResponse>(object); (void)pp;
    switch (field) {
        case FIELD_data: pp->setData(omnetpp::fromAnyPtr<::omnetpp::cMessage>(ptr)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSLookupResponse'", field);
    }
}

Register_Class(CSInsertRequest)

CSInsertRequest::CSInsertRequest(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(CS_INSERT_REQUEST);

    if (this->data != nullptr) take(this->data);
}

CSInsertRequest::CSInsertRequest(const CSInsertRequest& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

CSInsertRequest::~CSInsertRequest()
{
    dropAndDelete(this->data);
}

CSInsertRequest& CSInsertRequest::operator=(const CSInsertRequest& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void CSInsertRequest::copy(const CSInsertRequest& other)
{
    dropAndDelete(this->data);
    this->data = other.data;
    if (this->data != nullptr) {
        this->data = this->data->dup();
        take(this->data);
        this->data->setName(other.data->getName());
    }
}

void CSInsertRequest::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->data);
}

void CSInsertRequest::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->data);
}

const ::omnetpp::cMessage * CSInsertRequest::getData() const
{
    return this->data;
}

void CSInsertRequest::setData(::omnetpp::cMessage * data)
{
    if (data == this->data) return;
    if (this->data != nullptr) dropAndDelete(this->data);
    this->data = data;
    if (this->data != nullptr) take(this->data);
}

::omnetpp::cMessage * CSInsertRequest::removeData()
{
    ::omnetpp::cMessage * retval = this->data;
    if (retval != nullptr) drop(retval);
    this->data = nullptr;
    return retval;
}

class CSInsertRequestDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_data,
    };
  public:
    CSInsertRequestDescriptor();
    virtual ~CSInsertRequestDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(CSInsertRequestDescriptor)

CSInsertRequestDescriptor::CSInsertRequestDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::CSInsertRequest)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

CSInsertRequestDescriptor::~CSInsertRequestDescriptor()
{
    delete[] propertyNames;
}

bool CSInsertRequestDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<CSInsertRequest *>(obj)!=nullptr;
}

const char **CSInsertRequestDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *CSInsertRequestDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string CSInsertRequestDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void CSInsertRequestDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int CSInsertRequestDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 1+base->getFieldCount() : 1;
}

unsigned int CSInsertRequestDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND | FD_ISPOINTER | FD_ISCOBJECT | FD_ISCOWNEDOBJECT | FD_ISREPLACEABLE,    // FIELD_data
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *CSInsertRequestDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "data",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int CSInsertRequestDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "data") == 0) return baseIndex + 0;
    return base ? base->findField(fieldName) : -1;
}

const char *CSInsertRequestDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "omnetpp::cMessage",    // FIELD_data
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **CSInsertRequestDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_data: {
            static const char *names[] = { "owned", "allowReplace",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *CSInsertRequestDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_data:
            if (!strcmp(propertyName, "owned")) return "";
            if (!strcmp(propertyName, "allowReplace")) return "";
            return nullptr;
        default: return nullptr;
    }
}

int CSInsertRequestDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void CSInsertRequestDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'CSInsertRequest'", field);
    }
}

const char *CSInsertRequestDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    switch (field) {
        case FIELD_data: { const ::omnetpp::cMessage * value = pp->getData(); return omnetpp::opp_typename(typeid(*value)); }
        default: return nullptr;
    }
}

std::string CSInsertRequestDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    switch (field) {
        case FIELD_data: { auto obj = pp->getData(); return obj == nullptr ? "" : obj->str(); }
        default: return "";
    }
}

void CSInsertRequestDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSInsertRequest'", field);
    }
}

omnetpp::cValue CSInsertRequestDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    switch (field) {
        case FIELD_data: return omnetpp::toAnyPtr(pp->getData()); break;
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'CSInsertRequest' as cValue -- field index out of range?", field);
    }
}

void CSInsertRequestDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    switch (field) {
        case FIELD_data: pp->setData(omnetpp::fromAnyPtr<::omnetpp::cMessage>(value.pointerValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSInsertRequest'", field);
    }
}

const char *CSInsertRequestDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_data: return omnetpp::opp_typename(typeid(::omnetpp::cMessage));
        default: return nullptr;
    };
}

omnetpp::any_ptr CSInsertRequestDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    switch (field) {
        case FIELD_data: return omnetpp::toAnyPtr(pp->getData()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void CSInsertRequestDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    CSInsertRequest *pp = omnetpp::fromAnyPtr<CSInsertRequest>(object); (void)pp;
    switch (field) {
        case FIELD_data: pp->setData(omnetpp::fromAnyPtr<::omnetpp::cMessage>(ptr)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSInsertRequest'", field);
    }
}

Register_Class(CSInsertResponse)

CSInsertResponse::CSInsertResponse(const char *name, short kind) : ::ndniot::NDNControlMessage(name, kind)
{
    this->setMessageType(CS_INSERT_RESPONSE);

}

CSInsertResponse::CSInsertResponse(const CSInsertResponse& other) : ::ndniot::NDNControlMessage(other)
{
    copy(other);
}

CSInsertResponse::~CSInsertResponse()
{
}

CSInsertResponse& CSInsertResponse::operator=(const CSInsertResponse& other)
{
    if (this == &other) return *this;
    ::ndniot::NDNControlMessage::operator=(other);
    copy(other);
    return *this;
}

void CSInsertResponse::copy(const CSInsertResponse& other)
{
    this->success = other.success;
}

void CSInsertResponse::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NDNControlMessage::parsimPack(b);
    doParsimPacking(b,this->success);
}

void CSInsertResponse::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NDNControlMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->success);
}

bool CSInsertResponse::getSuccess() const
{
    return this->success;
}

void CSInsertResponse::setSuccess(bool success)
{
    this->success = success;
}

class CSInsertResponseDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_success,
    };
  public:
    CSInsertResponseDescriptor();
    virtual ~CSInsertResponseDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual std::string getValueAsString(omnetpp::any_ptr object) const override;
    virtual void setValueAsString(omnetpp::any_ptr object, const char *value) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(CSInsertResponseDescriptor)

CSInsertResponseDescriptor::CSInsertResponseDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::CSInsertResponse)), "ndniot::NDNControlMessage")
{
    propertyNames = nullptr;
}

CSInsertResponseDescriptor::~CSInsertResponseDescriptor()
{
    delete[] propertyNames;
}

bool CSInsertResponseDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<CSInsertResponse *>(obj)!=nullptr;
}

const char **CSInsertResponseDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *CSInsertResponseDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string CSInsertResponseDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void CSInsertResponseDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int CSInsertResponseDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 1+base->getFieldCount() : 1;
}

unsigned int CSInsertResponseDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_success
    };
    return (field >= 0 && field < 1) ? fieldTypeFlags[field] : 0;
}

const char *CSInsertResponseDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "success",
    };
    return (field >= 0 && field < 1) ? fieldNames[field] : nullptr;
}

int CSInsertResponseDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "success") == 0) return baseIndex + 0;
    return base ? base->findField(fieldName) : -1;
}

const char *CSInsertResponseDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "bool",    // FIELD_success
    };
    return (field >= 0 && field < 1) ? fieldTypeStrings[field] : nullptr;
}

const char **CSInsertResponseDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *CSInsertResponseDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int CSInsertResponseDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void CSInsertResponseDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'CSInsertResponse'", field);
    }
}

const char *CSInsertResponseDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string CSInsertResponseDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: return bool2string(pp->getSuccess());
        default: return "";
    }
}

void CSInsertResponseDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: pp->setSuccess(string2bool(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSInsertResponse'", field);
    }
}

omnetpp::cValue CSInsertResponseDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: return pp->getSuccess();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'CSInsertResponse' as cValue -- field index out of range?", field);
    }
}

void CSInsertResponseDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    switch (field) {
        case FIELD_success: pp->setSuccess(value.boolValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSInsertResponse'", field);
    }
}

const char *CSInsertResponseDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr CSInsertResponseDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void CSInsertResponseDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    CSInsertResponse *pp = omnetpp::fromAnyPtr<CSInsertResponse>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'CSInsertResponse'", field);
    }
}

}  // namespace ndniot

namespace omnetpp {

}  // namespace omnetpp

