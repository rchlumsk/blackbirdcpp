#ifndef VECTOR_H
#define VECTOR_H

#include "BlackbirdInclude.h"

class CVector {
public:
  // Member variables
  std::string name;                         // name of vector
  OGRSpatialReference* spat_ref;            // spatial reference of vector
  OGRwkbGeometryType geom_type;             // geometry type of vector
  std::vector<OGRFieldDefn*> field_defs;    // field definitions of vector
  std::vector<OGRFeature*> features;        // features of vector

  // Constructors and Destructor
  CVector();
  CVector(const CVector& other);
  ~CVector();

  // Copy assignment operator
  CVector& operator=(const CVector& other);

  // Member functions
  void add_to_field_def_map(const char* name, int ind); // adds pair to field_def_map
  int get_index_by_fieldname(const char* fieldname);    // returns index of field with field name "fieldname"

  // I/O Functions
  void WriteToFile(std::string filepath); // defined in StandardOutput.cpp - not yet implemented
  void pretty_print() const;              // defined in StandardOutput.cpp

protected:
  // Private variables
  std::unordered_map<std::string, int> field_def_map;   // maps field name to field_defs index

};

#endif
