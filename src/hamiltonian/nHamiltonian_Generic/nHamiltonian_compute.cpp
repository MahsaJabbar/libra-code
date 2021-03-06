/*********************************************************************************
* Copyright (C) 2017 Alexey V. Akimov
*
* This file is distributed under the terms of the GNU General Public License
* as published by the Free Software Foundation, either version 2 of
* the License, or (at your option) any later version.
* See the file LICENSE in the root directory of this distribution
* or <http://www.gnu.org/licenses/>.
*
*********************************************************************************/
/**
  \file nHamiltonian.cpp
  \brief The file implements the generic Hamiltonian class
    
*/


#include <stdlib.h>

#include "nHamiltonian.h"
#include "../../math_meigen/libmeigen.h"


/// liblibra namespace
namespace liblibra{

/// libhamiltonian namespace 
namespace libhamiltonian{

/// libhamiltonian_generic namespace 
namespace libhamiltonian_generic{

using namespace liblinalg;
using namespace libmeigen;



void nHamiltonian::compute_adiabatic(int lvl){
/**
  Compute the adiabatic Hamiltonian

  |psi_adi> = |psi_dia> * U

  Stationary SE:  H |psi_adi> =  |psi_adi> * E

  project on <psi_adi|, keeping in mind that <psi_adi|psi_adi> = I

  <psi_dia| H |psi_adi> =  <psi_dia|psi_adi> E  

  <psi_dia| H |psi_dia> * U =  <psi_dia|psi_dia> * U * E  

  H_dia * U = ovlp_dia * U * H_adi

  Here, U = basis_transform

  Assume: H does not depend on U

  lvl >= 0 - only diabatic - to - adiabatic transform
  lvl >= 1 - forces and derivative couplings
*/
  int i,j;
 
  if(lvl>=0){

    if(ham_dia_mem_status==0){ cout<<"Error in compute_adiabatic(): the diabatic Hamiltonian matrix is not allocated \
    but it is needed for the calculations\n"; exit(0); }
    if(ovlp_dia_mem_status==0){ cout<<"Error in compute_adiabatic(): the overlap matrix of the diabatic states is not allocated \
    but it is needed for the calculations\n"; exit(0); }

    if(ham_adi_mem_status==0){ cout<<"Error in compute_adiabatic(): the adiabatic Hamiltonian matrix is not allocated \
    but it is used to collect the results of the calculations\n"; exit(0); }
    if(basis_transform_mem_status==0){ cout<<"Error in compute_adiabatic(): the basis_transform (eigenvector) matrix is\
    not allocated but it is used to collect the results of the calculations\n"; exit(0); }

   
    solve_eigen(ham_dia, ovlp_dia, ham_adi, basis_transform, 0);  

    if(lvl>=1){

      // Now compute the derivative couplings (off-diagonal, multiplied by energy difference) and adiabatic gradients (diagonal)
      CMATRIX* tmp; tmp = new CMATRIX(nadi,nadi);
      CMATRIX* dtilda; dtilda = new CMATRIX(nadi,nadi);
      
      for(int n=0;n<nnucl;n++){

        if(d1ham_dia_mem_status[n]==0){ cout<<"Error in compute_adiabatic(): the derivatives of the diabatic Hamiltonian \
        matrix w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for calculations \n"; exit(0); }

        if(d1ham_adi_mem_status[n]==0){ cout<<"Error in compute_adiabatic(): the derivatives of the adiabatic Hamiltonian \
        matrix w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for collecting results \n"; exit(0); }


        if(dc1_dia_mem_status[n]==0){ cout<<"Error in compute_adiabatic(): the derivatives couplings matrix in the diabatic \
        basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for collecting results \n"; exit(0); }

        if(dc1_adi_mem_status[n]==0){ cout<<"Error in compute_adiabatic(): the derivatives couplings matrix in the adiabatic \
        basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for collecting results \n"; exit(0); }


        // E.g. see the derivations here: https://github.com/alexvakimov/Derivatory/blob/master/theory_NAC.pdf
        // also: http://www.theochem.ruhr-uni-bochum.de/~nikos.doltsinis/nic_10_doltsinis.pdf
        *tmp = (*basis_transform).H() * (*d1ham_dia[n]) * (*basis_transform);

        *dtilda = (*basis_transform) * (*dc1_dia[n]).H() * (*basis_transform).H() * (*ham_adi);
        *dtilda = (*dtilda + (*dtilda).H() );
        *tmp -= *dtilda;

        // Adiabatic "forces"
        *d1ham_adi[n] = 0.0;
        for(i=0;i<nadi;i++){ d1ham_adi[n]->set(i,i, tmp->get(i,i)); }

        // Adiabatic derivative couplings
        *dc1_adi[n] = 0.0;

        for(i=0;i<nadi;i++){
          for(j=0;j<nadi;j++){

            if(i==j){  dc1_adi[n]->set(i,j, 0.0, 0.0); }
            else{
 
              double dE = (ham_adi->get(j,j) - ham_adi->get(i,i) ).real();
              if(fabs(dE)<1e-10){ dE = 1e-10 * (dE>0.0 ? 1.0 : -1.0); }
          
              dc1_adi[n]->set(i,j, tmp->get(i,j)/dE );

            }
          }// for j
        }// for i
      }// for n

      delete tmp;
      delete dtilda;

    }// lvl>=1
  }// lvl>=0

     
}



void nHamiltonian::ampl_dia2adi(){
/**

  |PSI> = |psi_dia> * C_dia

  C_dia = U * C_adi

  U^H * S * U = I ==> U^-1 = U^H() * S, and C_adi = U^H() * S * C_dia

*/

  if(ovlp_dia_mem_status==0){ cout<<"Error in ampl_dia2adi(): the overlap matrix in the diabatic basis is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(basis_transform_mem_status==0){ cout<<"Error in ampl_dia2adi(): the transformation basis matrix is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ampl_dia_mem_status==0){ cout<<"Error in ampl_dia2adi(): the amplitudes of the diabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }

  if(ampl_adi_mem_status==0){ cout<<"Error in ampl_dia2adi(): the amplitudes of the adiabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }



  (*ampl_adi) = (*basis_transform).H() * (*ovlp_dia) * (*ampl_dia); 

}

void nHamiltonian::ampl_adi2dia(){
/**

  |PSI> = |psi_dia> * C_dia

  C_dia = U * C_adi


*/

  if(basis_transform_mem_status==0){ cout<<"Error in ampl_dia2adi(): the transformation basis matrix is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ampl_dia_mem_status==0){ cout<<"Error in ampl_dia2adi(): the amplitudes of the diabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }

  if(ampl_adi_mem_status==0){ cout<<"Error in ampl_dia2adi(): the amplitudes of the adiabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }



  (*ampl_dia) = (*basis_transform) * (*ampl_adi); 

}






vector<CMATRIX> nHamiltonian::forces_tens_adi(){
/**

  The dependence of U on R is neglected

  The elements of the returned list are the matrixes F_adi that, when multiplied
  by the adiabatic amplitudes give the derivative of the total energy in the adiabatic basis

  f_adi.M[n] = Cadi.H() * (F_adi[n]) * Cadi   = -dE/dR

  The normalization factor is embedded in F_adi[n]

*/

  vector<CMATRIX> res; res = vector<CMATRIX>(nnucl, CMATRIX(nadi,nadi));

  if(ampl_adi_mem_status==0){ cout<<"Error in forces_tens_adi(): the amplitudes of the adiabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }

  complex<double> norm = ((*ampl_adi).H() * (*ampl_adi)).M[0]; 


  for(int n=0;n<nnucl;n++){

    if(d1ham_adi_mem_status[n]==0){ cout<<"Error in forces_tens_adi(): the derivatives of the Hamiltonian matrix in the \
    adiabatic basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }

    res[n] = (-1.0/norm)*(*d1ham_adi[n]); 

  }// for n

  return res;

}


CMATRIX nHamiltonian::forces_adi(){
/**

  The dependence of U on R is neglected

  The elements of the returned matrix f_adi are the components 
  of the vector of the adiabatic force allong all DOFs

  Return : f_adi.M[n] = Cadi.H() * (F_adi[n]) * Cadi   = -dE/dR


*/


  CMATRIX res(nnucl, 1);
  vector<CMATRIX> dEdR;   dEdR = forces_tens_adi();

  if(ampl_adi_mem_status==0){ cout<<"Error in forces_adi(): the amplitudes of the adiabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }

  for(int n=0;n<nnucl;n++){  res.M[n] = ((*ampl_adi).H() * dEdR[n] * (*ampl_adi)).M[0];   }

  return res;
}




vector<CMATRIX> nHamiltonian::forces_tens_dia(){
/**

  The elements of the returned list are the matrixes F_dia that, when multiplied
  by the diabatic amplitudes give the derivative of the total energy in the diabatic basis

  f_dia.M[n] = Cdia.H() * (F_dia[n]) * Cdia   = -dE/dR

  The normalization factor is embedded in F_dia[n]

*/


  vector<CMATRIX> res; res = vector<CMATRIX>(nnucl, CMATRIX(ndia,ndia));

  if(ovlp_dia_mem_status==0){ cout<<"Error in forces_tens_dia(): the overlap matrix in the diabatic basis is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(basis_transform_mem_status==0){ cout<<"Error in forces_tens_dia(): the transformation basis matrix is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ampl_dia_mem_status==0){ cout<<"Error in forces_dia(): the amplitudes of the diabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }


  complex<double> norm = ((*ampl_dia).H() * (*ovlp_dia) * (*ampl_dia)).M[0]; 

  complex<double> Etot = ((*ampl_dia).H() * (*ham_dia) * (*ampl_dia)).M[0]/norm;



  for(int n=0;n<nnucl;n++){

    if(d1ham_dia_mem_status[n]==0){ cout<<"Error in forces_tens_dia(): the derivatives of the Hamiltonian matrix in the \
    adiabatic basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }

    if(dc1_dia_mem_status[n]==0){ cout<<"Error in forces_tens_dia(): the derivatives couplings matrix in the adiabatic \
    basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }


    res[n] = -1.0 * (*d1ham_dia[n] - Etot * (*dc1_dia[n] + (*dc1_dia[n]).H()) ); 

  }// for n


  return res;
}


CMATRIX nHamiltonian::forces_dia(){
/**

  The dependence of U on R is neglected

  The elements of the returned matrix f_dia are the components 
  of the vector of the diabatic force allong all DOFs

  Return : f_dia.M[n] = Cdia.H() * (F_dia[n]) * Cdia   = -dE/dR

*/


  CMATRIX res(nnucl, 1);
  vector<CMATRIX> dEdR;   dEdR = forces_tens_dia();

  if(ampl_dia_mem_status==0){ cout<<"Error in forces_dia(): the amplitudes of the diabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }

  for(int n=0;n<nnucl;n++){  res.M[n] = ((*ampl_dia).H() * dEdR[n] * (*ampl_dia)).M[0];   }

  return res;
}




complex<double> nHamiltonian::Ehrenfest_energy_adi(){
/**
  Compute the expectation value of the Hamiltonian in the adiabatic basis:

  This is an Ehrenfest energy for the superposition:

  |PSI> = |psi_adi> * C_adi 

  Here C_adi are the: 

  E = <PSI|H|PSI> = C_adi.H() * <psi_adi|H|psi_adi> * C_adi

*/

  if(ham_adi_mem_status==0){ cout<<"Error in Ehrenfest_energy_adi(): the adiabatic Hamiltonian matrix is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ampl_adi_mem_status==0){ cout<<"Error in Ehrenfest_energy_adi(): the amplitudes of the adiabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }


  complex<double> norm = ((*ampl_adi).H() * (*ampl_adi)).M[0]; 
  
  return ((*ampl_adi).H() * (*ham_adi) * (*ampl_adi)).M[0] / norm;

}



complex<double> nHamiltonian::Ehrenfest_energy_dia(){
/**
  Compute the expectation value of the Hamiltonian in the diabatic basis:

  This is an Ehrenfest energy for the superposition:

  |PSI> = |psi_dia> * C_dia 

  Here C_adi are the: 

  E = <PSI|H|PSI>/<PSI|PSI> = C_dia.H() * <psi_dia|H|psi_dia> * C_dia / C_dia.H() * S * C_dia

*/

  if(ovlp_dia_mem_status==0){ cout<<"Error in Ehrenfest_energy_dia(): the overlap matrix in the diabatic basis is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ham_dia_mem_status==0){ cout<<"Error in Ehrenfest_energy_dia(): the diabatic Hamiltonian matrix is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ampl_dia_mem_status==0){ cout<<"Error in Ehrenfest_energy_dia(): the amplitudes of the diabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }


  complex<double> norm = ((*ampl_dia).H() * (*ovlp_dia) * (*ampl_dia)).M[0]; 

  return ((*ampl_dia).H() * (*ham_dia) * (*ampl_dia)).M[0]/norm;

}



CMATRIX nHamiltonian::Ehrenfest_forces_adi(){
/**

  These are the Ehrenfest forces derived such the EOMs derived from the
  quntum-classical energy would conserve:

  H_qc = sum_i {p_i^2/2m_i} + <PSI|H|PSI>/<PSI|PSI>

  The wavefunction is expressed in the adiabatic basis:

  |PSI> = |psi_adi> * C_adi

  and evolves according to the TD-SE:

  i * hbar * d|PSI>/dt = H |PSI> 


  Some useful theory can be found here: 
  http://www.theochem.ruhr-uni-bochum.de/~nikos.doltsinis/nic_10_doltsinis.pdf

  for a systematic derivations, look here: 
  https://github.com/alexvakimov/Derivatory/blob/master/Ehrenfest.pdf

*/

  if(ham_adi_mem_status==0){ cout<<"Error in Ehrenfest_forces_adi(): the adiabatic Hamiltonian matrix is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ampl_adi_mem_status==0){ cout<<"Error in Ehrenfest_forces_adi(): the amplitudes of the adiabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }



  complex<double> norm = ((*ampl_adi).H() * (*ampl_adi)).M[0]; 

  CMATRIX res(nnucl,1);

  CMATRIX* tmp; tmp = new CMATRIX(nadi, nadi);


  for(int n=0;n<nnucl;n++){

    if(d1ham_adi_mem_status[n]==0){ cout<<"Error in Ehrenfest_forces_adi(): the derivatives of the Hamiltonian matrix in the \
    adiabatic basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }

    if(dc1_adi_mem_status[n]==0){ cout<<"Error in Ehrenfest_forces_adi(): the derivatives couplings matrix in the adiabatic \
    basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }


    *tmp = (*dc1_adi[n]).H() * (*ham_adi);
    *tmp = (*tmp + (*tmp).H() );

    res.M[n] = -((*ampl_adi).H() * (*d1ham_adi[n] - *tmp ) * (*ampl_adi) ).M[0];

  }// for n

  res /= norm; 
  delete tmp;

  return res;
}



CMATRIX nHamiltonian::Ehrenfest_forces_dia(){
/**

  These are the Ehrenfest forces derived such the EOMs derived from the
  quntum-classical energy would conserve:

  H_qc = sum_i {p_i^2/2m_i} + <PSI|H|PSI>/<PSI|PSI>

  The wavefunction is expressed in the diabatic basis:

  |PSI> = |psi_dia> * C_dia

  and evolves according to the TD-SE:

  i * hbar * d|PSI>/dt = H |PSI> 

  for a systematic derivations, look here: 
  https://github.com/alexvakimov/Derivatory/blob/master/Ehrenfest.pdf

*/

  if(ovlp_dia_mem_status==0){ cout<<"Error in Ehrenfest_forces_dia(): the overlap matrix in the diabatic basis is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ham_dia_mem_status==0){ cout<<"Error in Ehrenfest_forces_dia(): the diabatic Hamiltonian matrix is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ampl_dia_mem_status==0){ cout<<"Error in Ehrenfest_forces_dia(): the amplitudes of the diabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }


  CMATRIX res(nnucl,1);
  CMATRIX* dtilda; dtilda = new CMATRIX(nadi,nadi);
  CMATRIX* invS; invS = new CMATRIX(nadi, nadi); 

  FullPivLU_inverse(*ovlp_dia, *invS);

  complex<double> norm = ((*ampl_dia).H() * (*ovlp_dia) * (*ampl_dia)).M[0]; 

  
  for(int n=0;n<nnucl;n++){

      if(d1ham_dia_mem_status[n]==0){ cout<<"Error in Ehrenfest_forces_dia(): the derivatives of the Hamiltonian matrix in the \
      diabatic basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }

      if(dc1_dia_mem_status[n]==0){ cout<<"Error in Ehrenfest_forces_dia(): the derivatives couplings matrix in the diabatic \
      basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }


      *dtilda = (*dc1_dia[n]).H() * (*invS) * (*ham_dia);
      *dtilda = (*dtilda + (*dtilda).H() ) ;

      res.M[n] = -( (*ampl_dia).H() * (*d1ham_dia[n] - *dtilda ) * (*ampl_dia) ).M[0];

  }// for n

  res /= norm; 


  delete dtilda;
  delete invS;

  return res;
 
}




vector<CMATRIX> nHamiltonian::Ehrenfest_forces_tens_adi(){
/**
  The elements of the returned list are the matrixes F_adi^MF that, when multiplied
  by the adiabatic amplitudes give the Ehrenfest forces in the adiabatic basis

  f_adi^MF.M[n] = Cadi.H() * (F_adi^MF[n]) * Cadi  

  The normalization factor is embedded in F_adi^MF[n]
  
*/

  vector<CMATRIX> res; res = vector<CMATRIX>(nnucl, CMATRIX(nadi,nadi));

  if(ham_adi_mem_status==0){ cout<<"Error in Ehrenfest_forces_tens_adi(): the adiabatic Hamiltonian matrix is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ampl_adi_mem_status==0){ cout<<"Error in Ehrenfest_forces_tens_adi(): the amplitudes of the adiabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }



  complex<double> norm = ((*ampl_adi).H() * (*ampl_adi)).M[0]; 

  CMATRIX* tmp; tmp = new CMATRIX(nadi, nadi);


  for(int n=0;n<nnucl;n++){

    if(d1ham_adi_mem_status[n]==0){ cout<<"Error in Ehrenfest_forces_tens_adi(): the derivatives of the Hamiltonian matrix in the \
    adiabatic basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }

    if(dc1_adi_mem_status[n]==0){ cout<<"Error in Ehrenfest_forces_tens_adi(): the derivatives couplings matrix in the adiabatic \
    basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }

    *tmp = (*dc1_adi[n]).H() * (*ham_adi);
    *tmp = (*tmp + (*tmp).H() );

    res[n] = (-1.0/norm) * (*d1ham_adi[n] - *tmp );

  }// for n

  delete tmp;

  return res;

}


vector<CMATRIX> nHamiltonian::Ehrenfest_forces_tens_dia(){
/**
  The elements of the returned list are the matrixes F_dia^MF that, when multiplied
  by the diabatic amplitudes give the Ehrenfest forces in the diabatic basis

  f_dia^MF.M[n] = Cdia.H() * (F_dia^MF[n]) * Cdia  

  The normalization factor is embedded in F_dia^MF[n]

*/

  vector<CMATRIX> res; res = vector<CMATRIX>(nnucl, CMATRIX(ndia,ndia));

  if(ovlp_dia_mem_status==0){ cout<<"Error in Ehrenfest_forces_tens_dia(): the overlap matrix in the diabatic basis is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ham_dia_mem_status==0){ cout<<"Error in Ehrenfest_forces_tens_dia(): the diabatic Hamiltonian matrix is not allocated \
  but it is needed for the calculations\n"; exit(0); }

  if(ampl_dia_mem_status==0){ cout<<"Error in Ehrenfest_forces_tens_dia(): the amplitudes of the diabatic states are\
  not allocated, but they are needed for the calculations\n"; exit(0); }


  CMATRIX* dtilda; dtilda = new CMATRIX(nadi,nadi);
  CMATRIX* invS; invS = new CMATRIX(nadi, nadi); 

  FullPivLU_inverse(*ovlp_dia, *invS);
  complex<double> norm = ((*ampl_dia).H() * (*ovlp_dia) * (*ampl_dia)).M[0]; 

  
  for(int n=0;n<nnucl;n++){

      if(d1ham_dia_mem_status[n]==0){ cout<<"Error in Ehrenfest_forces_tens_dia(): the derivatives of the Hamiltonian matrix in the \
      diabatic basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }

      if(dc1_dia_mem_status[n]==0){ cout<<"Error in Ehrenfest_forces_tens_dia(): the derivatives couplings matrix in the diabatic \
      basis w.r.t. the nuclear DOF "<<n<<" is not allocated but is needed for the calculations \n"; exit(0); }


      *dtilda = (*dc1_dia[n]).H() * (*invS) * (*ham_dia);
      *dtilda = (*dtilda + (*dtilda).H() ) ;

      res[n] = (-1.0/norm) * (*d1ham_dia[n] - *dtilda );

  }// for n


  delete dtilda;
  delete invS;

  return res;
 
}






}// namespace libhamiltonian_generic
}// namespace libhamiltonian
}// liblibra

