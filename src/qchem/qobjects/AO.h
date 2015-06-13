#ifndef AO_H
#define AO_H

#include "../../mmath/libmmath.h"
using namespace libmmath;

#include "PrimitiveG.h"

namespace libqchem{
namespace libqobjects{


class AO{

// Objects of this class are atomic orbitals
// AO = linear combination of primitive gaussians (contraction)

public:

  // Members
  std::string element;              int is_element;
  std::string ao_shell;             int is_ao_shell;      // e.g. 1s, 2p, 3p, 3d, etc.
  std::string ao_shell_type;        int is_ao_shell_type; // e.g. s, p, d,  etc.
  std::string ao_name;              int is_ao_name;       // e.g. 1s, 2px, 3dxy,  etc.  int at_indx;

  // Angular momentum is defined for AOs, but each AO may have many Gaussians with different
  // alpha constants
  int x_exp;                        int is_x_exp;
  int y_exp;                        int is_y_exp;
  int z_exp;                        int is_z_exp;
  int expansion_size;               int is_expansion_size;
  vector<PrimitiveG> primitives;    int is_primitives;
  vector<double> coefficients;      int is_coefficients;  // these coefficients correspond to normalized primitive Gaussians



  //----------------- Function members --------------------
  AO();  ///   c-tor
  AO(const AO&); /// cc-tor
  AO& operator=(const AO&);

  // Construction and print  
  void init();
  void clear();
  void add_primitive(double c,PrimitiveG g);
  void show_info();

  // Computations
  double compute(VECTOR&);
  double norm2();
  double norm1();
  double normalization_factor();
  void normalize();


  // Transformations
  void shift_position(const VECTOR&);



  friend int operator == (const AO& g1, const AO& g2){
    int res = ((g1.x_exp==g2.x_exp) && (g1.y_exp==g2.y_exp) && (g1.z_exp==g2.z_exp)
              && (g1.expansion_size==g2.expansion_size) && (g1.ao_name==g2.ao_name)
              && (g1.ao_shell == g2.ao_shell) && (g1.ao_shell_type == g2.ao_shell_type) 
              && (g1.element == g2.element) 
              );
    if(res){ 
      for(int i=0;i<g1.expansion_size;i++){  res *= (g1.coefficients[i]==g2.coefficients[i]); }
    }
    if(res){ 
      for(int i=0;i<g1.expansion_size;i++){  res *= (g1.primitives[i]==g2.primitives[i]); }
    }
    return res;
  }


};


// Versions with references
double gaussian_overlap
( AO& AOa, AO& AOb,int is_normalize, int is_derivs,
  VECTOR& dIdA, VECTOR& dIdB, vector<double*>& auxd,int n_aux
);
double gaussian_overlap( AO& AOa, AO& AOb,int is_normalize, int is_derivs, VECTOR& dIdA, VECTOR& dIdB );
boost::python::list gaussian_overlap( AO& AOa, AO& AOb,int is_normalize, int is_derivs);
double gaussian_overlap(AO& AOa, AO& AOb,int is_normalize);
double gaussian_overlap(AO& AOa, AO& AOb);


// Versions with pointers - only for C++
double gaussian_overlap
( AO* AOa, AO* AOb,int is_normalize, int is_derivs,
  VECTOR& dIdA, VECTOR& dIdB, vector<double*>& auxd,int n_aux
);
double gaussian_overlap( AO* AOa, AO* AOb,int is_normalize, int is_derivs, VECTOR& dIdA, VECTOR& dIdB );
double gaussian_overlap(AO* AOa, AO* AOb,int is_normalize);
double gaussian_overlap(AO* AOa, AO* AOb);




typedef std::vector<AO> AOList;



}// namespace libqobjects
}// namespace libqchem



#endif // AO_H