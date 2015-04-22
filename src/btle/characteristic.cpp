#include "characteristic.h"

using namespace btle;

characteristic::characteristic()
: base(),
  uuid_(),
  attribute_handle_(0),
  characteristic_value_handle_(0),
  characteristic_properties_(0),
  instance_id_(0),
  descriptors_()
{
}

characteristic::characteristic(
    const std::string& data,
    const btle::uuid& uid)
: base(data),
  uuid_(uid),
  attribute_handle_(0),
  characteristic_value_handle_(0),
  characteristic_properties_(0),
  instance_id_(0),
  descriptors_()
{
}

characteristic::characteristic(
    const btle::uuid& uid)
: base(),
  uuid_(uid),
  attribute_handle_(0),
  characteristic_value_handle_(0),
  characteristic_properties_(0),
  instance_id_(0),
  descriptors_()
{
}

characteristic::characteristic(
    const btle::uuid& uid,
    uint8_t properties)
: base(),
  uuid_(uid),
  attribute_handle_(0),
  characteristic_value_handle_(0),
  characteristic_properties_(properties),
  instance_id_(0),
  descriptors_()
{
}

characteristic::characteristic(
    const btle::uuid& uid,
    uint8_t properties,
    long int instance_id)
: base(),
  uuid_(uid),
  attribute_handle_(0),
  characteristic_value_handle_(0),
  characteristic_properties_(properties),
  instance_id_(instance_id),
  descriptors_()
{
}

characteristic::characteristic(
    const btle::uuid& uid,
    uint8_t properties,
    uint16_t attribute_handle,
    uint16_t characteristic_properties)
: base(),
  uuid_(uid),
  attribute_handle_(attribute_handle),
  characteristic_value_handle_(0),
  characteristic_properties_(characteristic_properties),
  instance_id_(0),
  descriptors_()
{
}

characteristic::characteristic(const characteristic& other)
: base(other),
  uuid_(other.uuid_),
  attribute_handle_(other.attribute_handle_),
  characteristic_value_handle_(other.characteristic_value_handle_),
  characteristic_properties_(other.characteristic_properties_),
  instance_id_(other.instance_id_),
  descriptors_(other.descriptors_)
{
}

uint8_t characteristic::properties() const
{
    return characteristic_properties_;
}

const uuid& characteristic::uuid() const
{
    return uuid_;
}

uint8_t characteristic::attribute_handle() const
{
    return attribute_handle_;
}

long int characteristic::instance_id() const
{
    return instance_id_;
}

characteristic& characteristic::operator << (const descriptor& desc)
{
    // TODO add check for allready containing desc!
    descriptors_.push_back(desc);
    return *this;
}

bool characteristic::operator == (const characteristic& other) const
{
    return uuid_ == other.uuid() &&
           attribute_handle_ == other.attribute_handle();
}

bool characteristic::contains_descriptor_type(uint16_t type) const
{
    for( std::vector<descriptor>::const_iterator it = descriptors_.begin(); it != descriptors_.end(); ++it )
    {
        if( it->type() == type )
        {
            return true;
        }
    }
    return false;
}

descriptor* characteristic::descriptor_by_type(uint16_t type)
{
    for( std::vector<descriptor>::iterator it = descriptors_.begin(); it != descriptors_.end(); ++it )
    {
        if( it->type() == type )
        {
            return &(*it);
        }
    }
    return NULL;
}

std::string characteristic::to_string() const
{
    return uuid_.to_string();
}

