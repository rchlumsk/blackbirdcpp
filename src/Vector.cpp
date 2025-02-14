#include "BlackbirdInclude.h"
#include "Vector.h"

// Default Constructor
CVector::CVector()
  : name(PLACEHOLDER_STR),
  spat_ref(nullptr),
  geom_type(wkbUnknown),
  field_defs(),
  features() {
  // default constructor implementation
}

// Copy Constructor
CVector::CVector(const CVector& other)
  : name(other.name),
  geom_type(other.geom_type),
  field_def_map(other.field_def_map) {
  spat_ref = other.spat_ref ? other.spat_ref->Clone() : nullptr;

  for (const auto& field : other.field_defs) {
    field_defs.push_back(new OGRFieldDefn(field));
  }

  for (const auto& feature : other.features) {
    OGRFeature* feature_copy = OGRFeature::CreateFeature(feature->GetDefnRef());
    feature_copy->SetGeometry(feature->GetGeometryRef());
    feature_copy->SetFrom(feature);
    features.push_back(feature_copy);
  }
}

// Copy Assignment Operator (Copy-and-Swap)
CVector& CVector::operator=(const CVector& other) {
  if (this != &other) {
    CVector temp(other);

    std::swap(name, temp.name);
    std::swap(spat_ref, temp.spat_ref);
    std::swap(geom_type, temp.geom_type);
    std::swap(field_defs, temp.field_defs);
    std::swap(features, temp.features);
    std::swap(field_def_map, temp.field_def_map);
  }
  return *this;
}

//////////////////////////////////////////////////////////////////
/// \brief Returns index of field with field name 'fieldname'
/// \return index of field with field name 'fieldname'
//
int CVector::get_index_by_fieldname(const char* fieldname) {
  return field_def_map.find(fieldname) != field_def_map.end()
    ? field_def_map[fieldname]
    : PLACEHOLDER;
}

//////////////////////////////////////////////////////////////////
/// \brief Adds name and ind pair to field_def_map
//
void CVector::add_to_field_def_map(const char* name, int ind) {
  field_def_map[name] = ind;
}

// Destructor
CVector::~CVector() {
  if (spat_ref) {
    spat_ref->Release();
  }

  for (auto field : field_defs) {
    delete field;
  }
  field_defs.clear();

  for (auto feature : features) {
    OGRFeature::DestroyFeature(feature);
  }
  features.clear();
}