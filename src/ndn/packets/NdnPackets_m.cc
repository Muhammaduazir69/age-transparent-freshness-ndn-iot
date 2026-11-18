//
// Generated file, do not edit! Created by opp_msgtool 6.4 from ndn/packets/NdnPackets.msg.
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
#include "NdnPackets_m.h"

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

Register_Enum(ndniot::NdnPacketType, (ndniot::NdnPacketType::NDN_INTEREST, ndniot::NdnPacketType::NDN_DATA, ndniot::NdnPacketType::NDN_NACK));

InterestSelector::InterestSelector()
{
}

void __doPacking(omnetpp::cCommBuffer *b, const InterestSelector& a)
{
    doParsimPacking(b,a.minSuffixComponents);
    doParsimPacking(b,a.maxSuffixComponents);
    doParsimPacking(b,a.mustBeFresh);
}

void __doUnpacking(omnetpp::cCommBuffer *b, InterestSelector& a)
{
    doParsimUnpacking(b,a.minSuffixComponents);
    doParsimUnpacking(b,a.maxSuffixComponents);
    doParsimUnpacking(b,a.mustBeFresh);
}

class InterestSelectorDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_minSuffixComponents,
        FIELD_maxSuffixComponents,
        FIELD_mustBeFresh,
    };
  public:
    InterestSelectorDescriptor();
    virtual ~InterestSelectorDescriptor();

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

Register_ClassDescriptor(InterestSelectorDescriptor)

InterestSelectorDescriptor::InterestSelectorDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::InterestSelector)), "")
{
    propertyNames = nullptr;
}

InterestSelectorDescriptor::~InterestSelectorDescriptor()
{
    delete[] propertyNames;
}

bool InterestSelectorDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<InterestSelector *>(obj)!=nullptr;
}

const char **InterestSelectorDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *InterestSelectorDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string InterestSelectorDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    return toStringIfPrintable(*pp);
}

void InterestSelectorDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int InterestSelectorDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 3+base->getFieldCount() : 3;
}

unsigned int InterestSelectorDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_minSuffixComponents
        FD_ISEDITABLE,    // FIELD_maxSuffixComponents
        FD_ISEDITABLE,    // FIELD_mustBeFresh
    };
    return (field >= 0 && field < 3) ? fieldTypeFlags[field] : 0;
}

const char *InterestSelectorDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "minSuffixComponents",
        "maxSuffixComponents",
        "mustBeFresh",
    };
    return (field >= 0 && field < 3) ? fieldNames[field] : nullptr;
}

int InterestSelectorDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "minSuffixComponents") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "maxSuffixComponents") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "mustBeFresh") == 0) return baseIndex + 2;
    return base ? base->findField(fieldName) : -1;
}

const char *InterestSelectorDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_minSuffixComponents
        "int",    // FIELD_maxSuffixComponents
        "bool",    // FIELD_mustBeFresh
    };
    return (field >= 0 && field < 3) ? fieldTypeStrings[field] : nullptr;
}

const char **InterestSelectorDescriptor::getFieldPropertyNames(int field) const
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

const char *InterestSelectorDescriptor::getFieldProperty(int field, const char *propertyName) const
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

int InterestSelectorDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void InterestSelectorDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'InterestSelector'", field);
    }
}

const char *InterestSelectorDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string InterestSelectorDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    switch (field) {
        case FIELD_minSuffixComponents: return long2string(pp->minSuffixComponents);
        case FIELD_maxSuffixComponents: return long2string(pp->maxSuffixComponents);
        case FIELD_mustBeFresh: return bool2string(pp->mustBeFresh);
        default: return "";
    }
}

void InterestSelectorDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    switch (field) {
        case FIELD_minSuffixComponents: pp->minSuffixComponents = string2long(value); break;
        case FIELD_maxSuffixComponents: pp->maxSuffixComponents = string2long(value); break;
        case FIELD_mustBeFresh: pp->mustBeFresh = string2bool(value); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'InterestSelector'", field);
    }
}

omnetpp::cValue InterestSelectorDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    switch (field) {
        case FIELD_minSuffixComponents: return pp->minSuffixComponents;
        case FIELD_maxSuffixComponents: return pp->maxSuffixComponents;
        case FIELD_mustBeFresh: return pp->mustBeFresh;
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'InterestSelector' as cValue -- field index out of range?", field);
    }
}

void InterestSelectorDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    switch (field) {
        case FIELD_minSuffixComponents: pp->minSuffixComponents = omnetpp::checked_int_cast<int>(value.intValue()); break;
        case FIELD_maxSuffixComponents: pp->maxSuffixComponents = omnetpp::checked_int_cast<int>(value.intValue()); break;
        case FIELD_mustBeFresh: pp->mustBeFresh = value.boolValue(); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'InterestSelector'", field);
    }
}

const char *InterestSelectorDescriptor::getFieldStructName(int field) const
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

omnetpp::any_ptr InterestSelectorDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void InterestSelectorDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    InterestSelector *pp = omnetpp::fromAnyPtr<InterestSelector>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'InterestSelector'", field);
    }
}

Register_Enum(ndniot::NackReason, (ndniot::NackReason::NACK_NONE, ndniot::NackReason::NACK_CONGESTION, ndniot::NackReason::NACK_DUPLICATE, ndniot::NackReason::NACK_NO_ROUTE));

Register_Class(NdnPacket)

NdnPacket::NdnPacket(const char *name, short kind) : ::omnetpp::cPacket(name, kind)
{
}

NdnPacket::NdnPacket(const NdnPacket& other) : ::omnetpp::cPacket(other)
{
    copy(other);
}

NdnPacket::~NdnPacket()
{
}

NdnPacket& NdnPacket::operator=(const NdnPacket& other)
{
    if (this == &other) return *this;
    ::omnetpp::cPacket::operator=(other);
    copy(other);
    return *this;
}

void NdnPacket::copy(const NdnPacket& other)
{
    this->packetType = other.packetType;
    this->name = other.name;
    this->timestamp = other.timestamp;
    this->hopCount = other.hopCount;
    this->forwardingHint = other.forwardingHint;
    this->appPayload = other.appPayload;
}

void NdnPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cPacket::parsimPack(b);
    doParsimPacking(b,this->packetType);
    doParsimPacking(b,this->name);
    doParsimPacking(b,this->timestamp);
    doParsimPacking(b,this->hopCount);
    doParsimPacking(b,this->forwardingHint);
    doParsimPacking(b,this->appPayload);
}

void NdnPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->packetType);
    doParsimUnpacking(b,this->name);
    doParsimUnpacking(b,this->timestamp);
    doParsimUnpacking(b,this->hopCount);
    doParsimUnpacking(b,this->forwardingHint);
    doParsimUnpacking(b,this->appPayload);
}

int NdnPacket::getPacketType() const
{
    return this->packetType;
}

void NdnPacket::setPacketType(int packetType)
{
    this->packetType = packetType;
}

const char * NdnPacket::getName() const
{
    return this->name.c_str();
}

void NdnPacket::setName(const char * name)
{
    this->name = name;
}

::omnetpp::simtime_t NdnPacket::getTimestamp() const
{
    return this->timestamp;
}

void NdnPacket::setTimestamp(::omnetpp::simtime_t timestamp)
{
    this->timestamp = timestamp;
}

int NdnPacket::getHopCount() const
{
    return this->hopCount;
}

void NdnPacket::setHopCount(int hopCount)
{
    this->hopCount = hopCount;
}

const char * NdnPacket::getForwardingHint() const
{
    return this->forwardingHint.c_str();
}

void NdnPacket::setForwardingHint(const char * forwardingHint)
{
    this->forwardingHint = forwardingHint;
}

const char * NdnPacket::getAppPayload() const
{
    return this->appPayload.c_str();
}

void NdnPacket::setAppPayload(const char * appPayload)
{
    this->appPayload = appPayload;
}

class NdnPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_packetType,
        FIELD_name,
        FIELD_timestamp,
        FIELD_hopCount,
        FIELD_forwardingHint,
        FIELD_appPayload,
    };
  public:
    NdnPacketDescriptor();
    virtual ~NdnPacketDescriptor();

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

Register_ClassDescriptor(NdnPacketDescriptor)

NdnPacketDescriptor::NdnPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::NdnPacket)), "omnetpp::cPacket")
{
    propertyNames = nullptr;
}

NdnPacketDescriptor::~NdnPacketDescriptor()
{
    delete[] propertyNames;
}

bool NdnPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<NdnPacket *>(obj)!=nullptr;
}

const char **NdnPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *NdnPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string NdnPacketDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void NdnPacketDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int NdnPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 6+base->getFieldCount() : 6;
}

unsigned int NdnPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_packetType
        FD_ISEDITABLE,    // FIELD_name
        FD_ISEDITABLE,    // FIELD_timestamp
        FD_ISEDITABLE,    // FIELD_hopCount
        FD_ISEDITABLE,    // FIELD_forwardingHint
        FD_ISEDITABLE,    // FIELD_appPayload
    };
    return (field >= 0 && field < 6) ? fieldTypeFlags[field] : 0;
}

const char *NdnPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "packetType",
        "name",
        "timestamp",
        "hopCount",
        "forwardingHint",
        "appPayload",
    };
    return (field >= 0 && field < 6) ? fieldNames[field] : nullptr;
}

int NdnPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "packetType") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "name") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "timestamp") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "hopCount") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "forwardingHint") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "appPayload") == 0) return baseIndex + 5;
    return base ? base->findField(fieldName) : -1;
}

const char *NdnPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_packetType
        "string",    // FIELD_name
        "omnetpp::simtime_t",    // FIELD_timestamp
        "int",    // FIELD_hopCount
        "string",    // FIELD_forwardingHint
        "string",    // FIELD_appPayload
    };
    return (field >= 0 && field < 6) ? fieldTypeStrings[field] : nullptr;
}

const char **NdnPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_packetType: {
            static const char *names[] = { "enum", "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *NdnPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_packetType:
            if (!strcmp(propertyName, "enum")) return "NdnPacketType";
            if (!strcmp(propertyName, "enum")) return "ndniot::NdnPacketType";
            return nullptr;
        default: return nullptr;
    }
}

int NdnPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void NdnPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'NdnPacket'", field);
    }
}

const char *NdnPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string NdnPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    switch (field) {
        case FIELD_packetType: return enum2string(static_cast<int>(pp->getPacketType()), "ndniot::NdnPacketType");
        case FIELD_name: return oppstring2string(pp->getName());
        case FIELD_timestamp: return simtime2string(pp->getTimestamp());
        case FIELD_hopCount: return long2string(pp->getHopCount());
        case FIELD_forwardingHint: return oppstring2string(pp->getForwardingHint());
        case FIELD_appPayload: return oppstring2string(pp->getAppPayload());
        default: return "";
    }
}

void NdnPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    switch (field) {
        case FIELD_packetType: pp->setPacketType((ndniot::NdnPacketType)string2enum(value, "ndniot::NdnPacketType")); break;
        case FIELD_name: pp->setName((value)); break;
        case FIELD_timestamp: pp->setTimestamp(string2simtime(value)); break;
        case FIELD_hopCount: pp->setHopCount(string2long(value)); break;
        case FIELD_forwardingHint: pp->setForwardingHint((value)); break;
        case FIELD_appPayload: pp->setAppPayload((value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NdnPacket'", field);
    }
}

omnetpp::cValue NdnPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    switch (field) {
        case FIELD_packetType: return pp->getPacketType();
        case FIELD_name: return pp->getName();
        case FIELD_timestamp: return pp->getTimestamp().dbl();
        case FIELD_hopCount: return pp->getHopCount();
        case FIELD_forwardingHint: return pp->getForwardingHint();
        case FIELD_appPayload: return pp->getAppPayload();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'NdnPacket' as cValue -- field index out of range?", field);
    }
}

void NdnPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    switch (field) {
        case FIELD_packetType: pp->setPacketType((ndniot::NdnPacketType)value.intValue()); break;
        case FIELD_name: pp->setName(value.stringValue()); break;
        case FIELD_timestamp: pp->setTimestamp(value.doubleValue()); break;
        case FIELD_hopCount: pp->setHopCount(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_forwardingHint: pp->setForwardingHint(value.stringValue()); break;
        case FIELD_appPayload: pp->setAppPayload(value.stringValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NdnPacket'", field);
    }
}

const char *NdnPacketDescriptor::getFieldStructName(int field) const
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

omnetpp::any_ptr NdnPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void NdnPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    NdnPacket *pp = omnetpp::fromAnyPtr<NdnPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NdnPacket'", field);
    }
}

Register_Class(InterestPacket)

InterestPacket::InterestPacket(const char *name, short kind) : ::ndniot::NdnPacket(name, kind)
{
    this->setPacketType(NDN_INTEREST);

}

InterestPacket::InterestPacket(const InterestPacket& other) : ::ndniot::NdnPacket(other)
{
    copy(other);
}

InterestPacket::~InterestPacket()
{
}

InterestPacket& InterestPacket::operator=(const InterestPacket& other)
{
    if (this == &other) return *this;
    ::ndniot::NdnPacket::operator=(other);
    copy(other);
    return *this;
}

void InterestPacket::copy(const InterestPacket& other)
{
    this->nonce = other.nonce;
    this->interestLifetime = other.interestLifetime;
    this->selectors = other.selectors;
    this->contentType = other.contentType;
    this->priority = other.priority;
}

void InterestPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NdnPacket::parsimPack(b);
    doParsimPacking(b,this->nonce);
    doParsimPacking(b,this->interestLifetime);
    doParsimPacking(b,this->selectors);
    doParsimPacking(b,this->contentType);
    doParsimPacking(b,this->priority);
}

void InterestPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NdnPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->nonce);
    doParsimUnpacking(b,this->interestLifetime);
    doParsimUnpacking(b,this->selectors);
    doParsimUnpacking(b,this->contentType);
    doParsimUnpacking(b,this->priority);
}

int InterestPacket::getNonce() const
{
    return this->nonce;
}

void InterestPacket::setNonce(int nonce)
{
    this->nonce = nonce;
}

::omnetpp::simtime_t InterestPacket::getInterestLifetime() const
{
    return this->interestLifetime;
}

void InterestPacket::setInterestLifetime(::omnetpp::simtime_t interestLifetime)
{
    this->interestLifetime = interestLifetime;
}

const InterestSelector& InterestPacket::getSelectors() const
{
    return this->selectors;
}

void InterestPacket::setSelectors(const InterestSelector& selectors)
{
    this->selectors = selectors;
}

const char * InterestPacket::getContentType() const
{
    return this->contentType.c_str();
}

void InterestPacket::setContentType(const char * contentType)
{
    this->contentType = contentType;
}

int InterestPacket::getPriority() const
{
    return this->priority;
}

void InterestPacket::setPriority(int priority)
{
    this->priority = priority;
}

class InterestPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_nonce,
        FIELD_interestLifetime,
        FIELD_selectors,
        FIELD_contentType,
        FIELD_priority,
    };
  public:
    InterestPacketDescriptor();
    virtual ~InterestPacketDescriptor();

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

Register_ClassDescriptor(InterestPacketDescriptor)

InterestPacketDescriptor::InterestPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::InterestPacket)), "ndniot::NdnPacket")
{
    propertyNames = nullptr;
}

InterestPacketDescriptor::~InterestPacketDescriptor()
{
    delete[] propertyNames;
}

bool InterestPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<InterestPacket *>(obj)!=nullptr;
}

const char **InterestPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *InterestPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string InterestPacketDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void InterestPacketDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int InterestPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 5+base->getFieldCount() : 5;
}

unsigned int InterestPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_nonce
        FD_ISEDITABLE,    // FIELD_interestLifetime
        FD_ISCOMPOUND,    // FIELD_selectors
        FD_ISEDITABLE,    // FIELD_contentType
        FD_ISEDITABLE,    // FIELD_priority
    };
    return (field >= 0 && field < 5) ? fieldTypeFlags[field] : 0;
}

const char *InterestPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "nonce",
        "interestLifetime",
        "selectors",
        "contentType",
        "priority",
    };
    return (field >= 0 && field < 5) ? fieldNames[field] : nullptr;
}

int InterestPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "nonce") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "interestLifetime") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "selectors") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "contentType") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "priority") == 0) return baseIndex + 4;
    return base ? base->findField(fieldName) : -1;
}

const char *InterestPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_nonce
        "omnetpp::simtime_t",    // FIELD_interestLifetime
        "ndniot::InterestSelector",    // FIELD_selectors
        "string",    // FIELD_contentType
        "int",    // FIELD_priority
    };
    return (field >= 0 && field < 5) ? fieldTypeStrings[field] : nullptr;
}

const char **InterestPacketDescriptor::getFieldPropertyNames(int field) const
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

const char *InterestPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
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

int InterestPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void InterestPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'InterestPacket'", field);
    }
}

const char *InterestPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string InterestPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    switch (field) {
        case FIELD_nonce: return long2string(pp->getNonce());
        case FIELD_interestLifetime: return simtime2string(pp->getInterestLifetime());
        case FIELD_selectors: return "";
        case FIELD_contentType: return oppstring2string(pp->getContentType());
        case FIELD_priority: return long2string(pp->getPriority());
        default: return "";
    }
}

void InterestPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    switch (field) {
        case FIELD_nonce: pp->setNonce(string2long(value)); break;
        case FIELD_interestLifetime: pp->setInterestLifetime(string2simtime(value)); break;
        case FIELD_contentType: pp->setContentType((value)); break;
        case FIELD_priority: pp->setPriority(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'InterestPacket'", field);
    }
}

omnetpp::cValue InterestPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    switch (field) {
        case FIELD_nonce: return pp->getNonce();
        case FIELD_interestLifetime: return pp->getInterestLifetime().dbl();
        case FIELD_selectors: return omnetpp::toAnyPtr(&pp->getSelectors()); break;
        case FIELD_contentType: return pp->getContentType();
        case FIELD_priority: return pp->getPriority();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'InterestPacket' as cValue -- field index out of range?", field);
    }
}

void InterestPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    switch (field) {
        case FIELD_nonce: pp->setNonce(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_interestLifetime: pp->setInterestLifetime(value.doubleValue()); break;
        case FIELD_contentType: pp->setContentType(value.stringValue()); break;
        case FIELD_priority: pp->setPriority(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'InterestPacket'", field);
    }
}

const char *InterestPacketDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_selectors: return omnetpp::opp_typename(typeid(InterestSelector));
        default: return nullptr;
    };
}

omnetpp::any_ptr InterestPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    switch (field) {
        case FIELD_selectors: return omnetpp::toAnyPtr(&pp->getSelectors()); break;
        default: return omnetpp::any_ptr(nullptr);
    }
}

void InterestPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    InterestPacket *pp = omnetpp::fromAnyPtr<InterestPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'InterestPacket'", field);
    }
}

Register_Class(DataPacket)

DataPacket::DataPacket(const char *name, short kind) : ::ndniot::NdnPacket(name, kind)
{
    this->setPacketType(NDN_DATA);

}

DataPacket::DataPacket(const DataPacket& other) : ::ndniot::NdnPacket(other)
{
    copy(other);
}

DataPacket::~DataPacket()
{
}

DataPacket& DataPacket::operator=(const DataPacket& other)
{
    if (this == &other) return *this;
    ::ndniot::NdnPacket::operator=(other);
    copy(other);
    return *this;
}

void DataPacket::copy(const DataPacket& other)
{
    this->content = other.content;
    this->contentLength = other.contentLength;
    this->freshnessPeriod = other.freshnessPeriod;
    this->originTime = other.originTime;
    this->finalBlockId = other.finalBlockId;
    this->contentType = other.contentType;
    this->isSigned_ = other.isSigned_;
    this->signature = other.signature;
    this->signerId = other.signerId;
    this->signatureTime = other.signatureTime;
    this->isCacheable_ = other.isCacheable_;
    this->cacheHits = other.cacheHits;
    this->lcdDepth = other.lcdDepth;
    this->servedFromCache = other.servedFromCache;
    this->restamps = other.restamps;
    this->nominalFreshness = other.nominalFreshness;
}

void DataPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::NdnPacket::parsimPack(b);
    doParsimPacking(b,this->content);
    doParsimPacking(b,this->contentLength);
    doParsimPacking(b,this->freshnessPeriod);
    doParsimPacking(b,this->originTime);
    doParsimPacking(b,this->finalBlockId);
    doParsimPacking(b,this->contentType);
    doParsimPacking(b,this->isSigned_);
    doParsimPacking(b,this->signature);
    doParsimPacking(b,this->signerId);
    doParsimPacking(b,this->signatureTime);
    doParsimPacking(b,this->isCacheable_);
    doParsimPacking(b,this->cacheHits);
    doParsimPacking(b,this->lcdDepth);
    doParsimPacking(b,this->servedFromCache);
    doParsimPacking(b,this->restamps);
    doParsimPacking(b,this->nominalFreshness);
}

void DataPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::NdnPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->content);
    doParsimUnpacking(b,this->contentLength);
    doParsimUnpacking(b,this->freshnessPeriod);
    doParsimUnpacking(b,this->originTime);
    doParsimUnpacking(b,this->finalBlockId);
    doParsimUnpacking(b,this->contentType);
    doParsimUnpacking(b,this->isSigned_);
    doParsimUnpacking(b,this->signature);
    doParsimUnpacking(b,this->signerId);
    doParsimUnpacking(b,this->signatureTime);
    doParsimUnpacking(b,this->isCacheable_);
    doParsimUnpacking(b,this->cacheHits);
    doParsimUnpacking(b,this->lcdDepth);
    doParsimUnpacking(b,this->servedFromCache);
    doParsimUnpacking(b,this->restamps);
    doParsimUnpacking(b,this->nominalFreshness);
}

const char * DataPacket::getContent() const
{
    return this->content.c_str();
}

void DataPacket::setContent(const char * content)
{
    this->content = content;
}

int DataPacket::getContentLength() const
{
    return this->contentLength;
}

void DataPacket::setContentLength(int contentLength)
{
    this->contentLength = contentLength;
}

::omnetpp::simtime_t DataPacket::getFreshnessPeriod() const
{
    return this->freshnessPeriod;
}

void DataPacket::setFreshnessPeriod(::omnetpp::simtime_t freshnessPeriod)
{
    this->freshnessPeriod = freshnessPeriod;
}

::omnetpp::simtime_t DataPacket::getOriginTime() const
{
    return this->originTime;
}

void DataPacket::setOriginTime(::omnetpp::simtime_t originTime)
{
    this->originTime = originTime;
}

int DataPacket::getFinalBlockId() const
{
    return this->finalBlockId;
}

void DataPacket::setFinalBlockId(int finalBlockId)
{
    this->finalBlockId = finalBlockId;
}

const char * DataPacket::getContentType() const
{
    return this->contentType.c_str();
}

void DataPacket::setContentType(const char * contentType)
{
    this->contentType = contentType;
}

bool DataPacket::isSigned() const
{
    return this->isSigned_;
}

void DataPacket::setIsSigned(bool isSigned)
{
    this->isSigned_ = isSigned;
}

const char * DataPacket::getSignature() const
{
    return this->signature.c_str();
}

void DataPacket::setSignature(const char * signature)
{
    this->signature = signature;
}

const char * DataPacket::getSignerId() const
{
    return this->signerId.c_str();
}

void DataPacket::setSignerId(const char * signerId)
{
    this->signerId = signerId;
}

::omnetpp::simtime_t DataPacket::getSignatureTime() const
{
    return this->signatureTime;
}

void DataPacket::setSignatureTime(::omnetpp::simtime_t signatureTime)
{
    this->signatureTime = signatureTime;
}

bool DataPacket::isCacheable() const
{
    return this->isCacheable_;
}

void DataPacket::setIsCacheable(bool isCacheable)
{
    this->isCacheable_ = isCacheable;
}

int DataPacket::getCacheHits() const
{
    return this->cacheHits;
}

void DataPacket::setCacheHits(int cacheHits)
{
    this->cacheHits = cacheHits;
}

int DataPacket::getLcdDepth() const
{
    return this->lcdDepth;
}

void DataPacket::setLcdDepth(int lcdDepth)
{
    this->lcdDepth = lcdDepth;
}

bool DataPacket::getServedFromCache() const
{
    return this->servedFromCache;
}

void DataPacket::setServedFromCache(bool servedFromCache)
{
    this->servedFromCache = servedFromCache;
}

int DataPacket::getRestamps() const
{
    return this->restamps;
}

void DataPacket::setRestamps(int restamps)
{
    this->restamps = restamps;
}

::omnetpp::simtime_t DataPacket::getNominalFreshness() const
{
    return this->nominalFreshness;
}

void DataPacket::setNominalFreshness(::omnetpp::simtime_t nominalFreshness)
{
    this->nominalFreshness = nominalFreshness;
}

class DataPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_content,
        FIELD_contentLength,
        FIELD_freshnessPeriod,
        FIELD_originTime,
        FIELD_finalBlockId,
        FIELD_contentType,
        FIELD_isSigned,
        FIELD_signature,
        FIELD_signerId,
        FIELD_signatureTime,
        FIELD_isCacheable,
        FIELD_cacheHits,
        FIELD_lcdDepth,
        FIELD_servedFromCache,
        FIELD_restamps,
        FIELD_nominalFreshness,
    };
  public:
    DataPacketDescriptor();
    virtual ~DataPacketDescriptor();

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

Register_ClassDescriptor(DataPacketDescriptor)

DataPacketDescriptor::DataPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::DataPacket)), "ndniot::NdnPacket")
{
    propertyNames = nullptr;
}

DataPacketDescriptor::~DataPacketDescriptor()
{
    delete[] propertyNames;
}

bool DataPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<DataPacket *>(obj)!=nullptr;
}

const char **DataPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *DataPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string DataPacketDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void DataPacketDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int DataPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 16+base->getFieldCount() : 16;
}

unsigned int DataPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_content
        FD_ISEDITABLE,    // FIELD_contentLength
        FD_ISEDITABLE,    // FIELD_freshnessPeriod
        FD_ISEDITABLE,    // FIELD_originTime
        FD_ISEDITABLE,    // FIELD_finalBlockId
        FD_ISEDITABLE,    // FIELD_contentType
        FD_ISEDITABLE,    // FIELD_isSigned
        FD_ISEDITABLE,    // FIELD_signature
        FD_ISEDITABLE,    // FIELD_signerId
        FD_ISEDITABLE,    // FIELD_signatureTime
        FD_ISEDITABLE,    // FIELD_isCacheable
        FD_ISEDITABLE,    // FIELD_cacheHits
        FD_ISEDITABLE,    // FIELD_lcdDepth
        FD_ISEDITABLE,    // FIELD_servedFromCache
        FD_ISEDITABLE,    // FIELD_restamps
        FD_ISEDITABLE,    // FIELD_nominalFreshness
    };
    return (field >= 0 && field < 16) ? fieldTypeFlags[field] : 0;
}

const char *DataPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "content",
        "contentLength",
        "freshnessPeriod",
        "originTime",
        "finalBlockId",
        "contentType",
        "isSigned",
        "signature",
        "signerId",
        "signatureTime",
        "isCacheable",
        "cacheHits",
        "lcdDepth",
        "servedFromCache",
        "restamps",
        "nominalFreshness",
    };
    return (field >= 0 && field < 16) ? fieldNames[field] : nullptr;
}

int DataPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "content") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "contentLength") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "freshnessPeriod") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "originTime") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "finalBlockId") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "contentType") == 0) return baseIndex + 5;
    if (strcmp(fieldName, "isSigned") == 0) return baseIndex + 6;
    if (strcmp(fieldName, "signature") == 0) return baseIndex + 7;
    if (strcmp(fieldName, "signerId") == 0) return baseIndex + 8;
    if (strcmp(fieldName, "signatureTime") == 0) return baseIndex + 9;
    if (strcmp(fieldName, "isCacheable") == 0) return baseIndex + 10;
    if (strcmp(fieldName, "cacheHits") == 0) return baseIndex + 11;
    if (strcmp(fieldName, "lcdDepth") == 0) return baseIndex + 12;
    if (strcmp(fieldName, "servedFromCache") == 0) return baseIndex + 13;
    if (strcmp(fieldName, "restamps") == 0) return baseIndex + 14;
    if (strcmp(fieldName, "nominalFreshness") == 0) return baseIndex + 15;
    return base ? base->findField(fieldName) : -1;
}

const char *DataPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "string",    // FIELD_content
        "int",    // FIELD_contentLength
        "omnetpp::simtime_t",    // FIELD_freshnessPeriod
        "omnetpp::simtime_t",    // FIELD_originTime
        "int",    // FIELD_finalBlockId
        "string",    // FIELD_contentType
        "bool",    // FIELD_isSigned
        "string",    // FIELD_signature
        "string",    // FIELD_signerId
        "omnetpp::simtime_t",    // FIELD_signatureTime
        "bool",    // FIELD_isCacheable
        "int",    // FIELD_cacheHits
        "int",    // FIELD_lcdDepth
        "bool",    // FIELD_servedFromCache
        "int",    // FIELD_restamps
        "omnetpp::simtime_t",    // FIELD_nominalFreshness
    };
    return (field >= 0 && field < 16) ? fieldTypeStrings[field] : nullptr;
}

const char **DataPacketDescriptor::getFieldPropertyNames(int field) const
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

const char *DataPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
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

int DataPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void DataPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'DataPacket'", field);
    }
}

const char *DataPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string DataPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    switch (field) {
        case FIELD_content: return oppstring2string(pp->getContent());
        case FIELD_contentLength: return long2string(pp->getContentLength());
        case FIELD_freshnessPeriod: return simtime2string(pp->getFreshnessPeriod());
        case FIELD_originTime: return simtime2string(pp->getOriginTime());
        case FIELD_finalBlockId: return long2string(pp->getFinalBlockId());
        case FIELD_contentType: return oppstring2string(pp->getContentType());
        case FIELD_isSigned: return bool2string(pp->isSigned());
        case FIELD_signature: return oppstring2string(pp->getSignature());
        case FIELD_signerId: return oppstring2string(pp->getSignerId());
        case FIELD_signatureTime: return simtime2string(pp->getSignatureTime());
        case FIELD_isCacheable: return bool2string(pp->isCacheable());
        case FIELD_cacheHits: return long2string(pp->getCacheHits());
        case FIELD_lcdDepth: return long2string(pp->getLcdDepth());
        case FIELD_servedFromCache: return bool2string(pp->getServedFromCache());
        case FIELD_restamps: return long2string(pp->getRestamps());
        case FIELD_nominalFreshness: return simtime2string(pp->getNominalFreshness());
        default: return "";
    }
}

void DataPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    switch (field) {
        case FIELD_content: pp->setContent((value)); break;
        case FIELD_contentLength: pp->setContentLength(string2long(value)); break;
        case FIELD_freshnessPeriod: pp->setFreshnessPeriod(string2simtime(value)); break;
        case FIELD_originTime: pp->setOriginTime(string2simtime(value)); break;
        case FIELD_finalBlockId: pp->setFinalBlockId(string2long(value)); break;
        case FIELD_contentType: pp->setContentType((value)); break;
        case FIELD_isSigned: pp->setIsSigned(string2bool(value)); break;
        case FIELD_signature: pp->setSignature((value)); break;
        case FIELD_signerId: pp->setSignerId((value)); break;
        case FIELD_signatureTime: pp->setSignatureTime(string2simtime(value)); break;
        case FIELD_isCacheable: pp->setIsCacheable(string2bool(value)); break;
        case FIELD_cacheHits: pp->setCacheHits(string2long(value)); break;
        case FIELD_lcdDepth: pp->setLcdDepth(string2long(value)); break;
        case FIELD_servedFromCache: pp->setServedFromCache(string2bool(value)); break;
        case FIELD_restamps: pp->setRestamps(string2long(value)); break;
        case FIELD_nominalFreshness: pp->setNominalFreshness(string2simtime(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'DataPacket'", field);
    }
}

omnetpp::cValue DataPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    switch (field) {
        case FIELD_content: return pp->getContent();
        case FIELD_contentLength: return pp->getContentLength();
        case FIELD_freshnessPeriod: return pp->getFreshnessPeriod().dbl();
        case FIELD_originTime: return pp->getOriginTime().dbl();
        case FIELD_finalBlockId: return pp->getFinalBlockId();
        case FIELD_contentType: return pp->getContentType();
        case FIELD_isSigned: return pp->isSigned();
        case FIELD_signature: return pp->getSignature();
        case FIELD_signerId: return pp->getSignerId();
        case FIELD_signatureTime: return pp->getSignatureTime().dbl();
        case FIELD_isCacheable: return pp->isCacheable();
        case FIELD_cacheHits: return pp->getCacheHits();
        case FIELD_lcdDepth: return pp->getLcdDepth();
        case FIELD_servedFromCache: return pp->getServedFromCache();
        case FIELD_restamps: return pp->getRestamps();
        case FIELD_nominalFreshness: return pp->getNominalFreshness().dbl();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'DataPacket' as cValue -- field index out of range?", field);
    }
}

void DataPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    switch (field) {
        case FIELD_content: pp->setContent(value.stringValue()); break;
        case FIELD_contentLength: pp->setContentLength(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_freshnessPeriod: pp->setFreshnessPeriod(value.doubleValue()); break;
        case FIELD_originTime: pp->setOriginTime(value.doubleValue()); break;
        case FIELD_finalBlockId: pp->setFinalBlockId(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_contentType: pp->setContentType(value.stringValue()); break;
        case FIELD_isSigned: pp->setIsSigned(value.boolValue()); break;
        case FIELD_signature: pp->setSignature(value.stringValue()); break;
        case FIELD_signerId: pp->setSignerId(value.stringValue()); break;
        case FIELD_signatureTime: pp->setSignatureTime(value.doubleValue()); break;
        case FIELD_isCacheable: pp->setIsCacheable(value.boolValue()); break;
        case FIELD_cacheHits: pp->setCacheHits(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_lcdDepth: pp->setLcdDepth(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_servedFromCache: pp->setServedFromCache(value.boolValue()); break;
        case FIELD_restamps: pp->setRestamps(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_nominalFreshness: pp->setNominalFreshness(value.doubleValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'DataPacket'", field);
    }
}

const char *DataPacketDescriptor::getFieldStructName(int field) const
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

omnetpp::any_ptr DataPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void DataPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    DataPacket *pp = omnetpp::fromAnyPtr<DataPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'DataPacket'", field);
    }
}

Register_Class(NackPacket)

NackPacket::NackPacket(const char *name, short kind) : ::ndniot::InterestPacket(name, kind)
{
    this->setPacketType(NDN_NACK);

}

NackPacket::NackPacket(const NackPacket& other) : ::ndniot::InterestPacket(other)
{
    copy(other);
}

NackPacket::~NackPacket()
{
}

NackPacket& NackPacket::operator=(const NackPacket& other)
{
    if (this == &other) return *this;
    ::ndniot::InterestPacket::operator=(other);
    copy(other);
    return *this;
}

void NackPacket::copy(const NackPacket& other)
{
    this->reason = other.reason;
    this->reasonText = other.reasonText;
}

void NackPacket::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::ndniot::InterestPacket::parsimPack(b);
    doParsimPacking(b,this->reason);
    doParsimPacking(b,this->reasonText);
}

void NackPacket::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::ndniot::InterestPacket::parsimUnpack(b);
    doParsimUnpacking(b,this->reason);
    doParsimUnpacking(b,this->reasonText);
}

int NackPacket::getReason() const
{
    return this->reason;
}

void NackPacket::setReason(int reason)
{
    this->reason = reason;
}

const char * NackPacket::getReasonText() const
{
    return this->reasonText.c_str();
}

void NackPacket::setReasonText(const char * reasonText)
{
    this->reasonText = reasonText;
}

class NackPacketDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_reason,
        FIELD_reasonText,
    };
  public:
    NackPacketDescriptor();
    virtual ~NackPacketDescriptor();

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

Register_ClassDescriptor(NackPacketDescriptor)

NackPacketDescriptor::NackPacketDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(ndniot::NackPacket)), "ndniot::InterestPacket")
{
    propertyNames = nullptr;
}

NackPacketDescriptor::~NackPacketDescriptor()
{
    delete[] propertyNames;
}

bool NackPacketDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<NackPacket *>(obj)!=nullptr;
}

const char **NackPacketDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *NackPacketDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

std::string NackPacketDescriptor::getValueAsString(omnetpp::any_ptr object) const
{
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    return ((cObject*)pp)->str();
}

void NackPacketDescriptor::setValueAsString(omnetpp::any_ptr object, const char *value) const
{
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    if (!fromStringIfExtractable(*pp, value))
        cClassDescriptor::setValueAsString(object, value);
}

int NackPacketDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 2+base->getFieldCount() : 2;
}

unsigned int NackPacketDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_reason
        FD_ISEDITABLE,    // FIELD_reasonText
    };
    return (field >= 0 && field < 2) ? fieldTypeFlags[field] : 0;
}

const char *NackPacketDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "reason",
        "reasonText",
    };
    return (field >= 0 && field < 2) ? fieldNames[field] : nullptr;
}

int NackPacketDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "reason") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "reasonText") == 0) return baseIndex + 1;
    return base ? base->findField(fieldName) : -1;
}

const char *NackPacketDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_reason
        "string",    // FIELD_reasonText
    };
    return (field >= 0 && field < 2) ? fieldTypeStrings[field] : nullptr;
}

const char **NackPacketDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_reason: {
            static const char *names[] = { "enum", "enum",  nullptr };
            return names;
        }
        default: return nullptr;
    }
}

const char *NackPacketDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        case FIELD_reason:
            if (!strcmp(propertyName, "enum")) return "NackReason";
            if (!strcmp(propertyName, "enum")) return "ndniot::NackReason";
            return nullptr;
        default: return nullptr;
    }
}

int NackPacketDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void NackPacketDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'NackPacket'", field);
    }
}

const char *NackPacketDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string NackPacketDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    switch (field) {
        case FIELD_reason: return enum2string(static_cast<int>(pp->getReason()), "ndniot::NackReason");
        case FIELD_reasonText: return oppstring2string(pp->getReasonText());
        default: return "";
    }
}

void NackPacketDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    switch (field) {
        case FIELD_reason: pp->setReason((ndniot::NackReason)string2enum(value, "ndniot::NackReason")); break;
        case FIELD_reasonText: pp->setReasonText((value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NackPacket'", field);
    }
}

omnetpp::cValue NackPacketDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    switch (field) {
        case FIELD_reason: return pp->getReason();
        case FIELD_reasonText: return pp->getReasonText();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'NackPacket' as cValue -- field index out of range?", field);
    }
}

void NackPacketDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    switch (field) {
        case FIELD_reason: pp->setReason((ndniot::NackReason)value.intValue()); break;
        case FIELD_reasonText: pp->setReasonText(value.stringValue()); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NackPacket'", field);
    }
}

const char *NackPacketDescriptor::getFieldStructName(int field) const
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

omnetpp::any_ptr NackPacketDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void NackPacketDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    NackPacket *pp = omnetpp::fromAnyPtr<NackPacket>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'NackPacket'", field);
    }
}

}  // namespace ndniot

namespace omnetpp {

}  // namespace omnetpp

