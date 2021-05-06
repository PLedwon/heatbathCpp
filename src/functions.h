#ifndef functions
#define functions
#include <cmath>
#include <iostream>
#include <random>
#include <fstream>
#include <array>
using std::array;



























template<typename T,size_t n>
void printArray_(array<T,n> a) {
    int i;
    for (i = a.front(); i < a.size(); i++) {
        //std::cout << a[i] << ' ';
        printf("%e ", a[i]);
    }
    std::cout << '\n';
}

template<size_t n>
double avg(array<double,n> a) {
    double sum=0;
    for (int i = a.front(); i < a.size(); ++i) {
       sum += a[i];
    }
    return (double) sum/a.size();
}

template<size_t n>
void computeMasses(array<double,n> (&masses), double oscMass, double M, array<double, n-1> omega, double omegaMin, const double GAMMA){
  masses[0]=M;
  for (int i = 1; i < masses.size() ; i++) {
    masses[i]=oscMass*pow((omega[i-1]/omegaMin),(GAMMA-3))*exp(-omega[i-1]);
  }

}

template<size_t n>
void computeSpringConstants(array<double,n> (&k), array<double,n> masses, array<double,n-1> omega) {
  k[0] = 0;
  for (int i = 1; i < k.size(); i++) {
    k[i]=masses[i]*pow(omega[i-1],2);
  }
}
/*
double H(heatbath bath, const array<double> k, const array<double> invM) { // compute total energy of the system
  double E = bath.p[0] * bath.p[0] * invM[0];
  for (int i = 1; i < bath.q.size() ; ++i) {
    E += bath.p[i] * bath.p[i] * invM[i] + k[i] * pow(bath.q[i] - bath.q[0], 2);
    //printf("accumulated energy %e \n", E);
    }
  E *= 0.5;
  return  E;
}
*/
template<size_t n>
double sum(array <double,n> p) {
    double mom = 0;
    for (int i = p.front(); i < p.size(); ++i) {
        mom += p[i];
    }
    return mom;
}

template< size_t n>
void setEigenfrequencies(array<double,n> (&omega), double omegaMin, double omegaMax) {
    double c;
    c = (omegaMax - omegaMin)/(omega.size()-1);
    for(int i = 0; i < omega.size() ; ++i) // equidistant distribution of eigenfrequencies of the harmonic oscillators
        omega[i] = omegaMin + i*c;
    //omega.back() = omegaMax;
}

template< size_t n>
void invertMasses(array<double,n> (&invM), array<double,n> masses) {
    for (int i = 0; i < invM.size(); ++i) {
       invM[i] = 1/masses[i];
    }
}

template<size_t n>
void generateInitialConditions(array<double,n> &q,  array<double,n> &p, double M,  array<double,n> masses, array<double,n> k, const double BETA) {

    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<> d{0,1};
    double pref=pow(BETA,-0.5);

    //set the initial conditions for the distinguished particle
    q[0] = 0;
    p[0] = pref * pow(M,0.5) * d(gen);

    for (int i = 1; i < q.size(); ++i) {
       q[i] = q[0] + pref*pow(k[i],-0.5)*d(gen);
       p[i] = pref*pow(masses[i],0.5)* d(gen);
    }
    //initialize heatbath with vanishing center of mass velocity
    double avgMomentum = avg(p);
    double psum = sum(p);
    for (int i = 0; i < p.size(); ++i) {
        p[i] -= avgMomentum;
    }
    psum = sum(p);

    //check that total momentum is close to zero
    if (std::abs(psum) > pow(10,-13)) {
        throw "Error: CoM velocity is not 0 while initializing heatbath";
    }
}



template< size_t n>
void updateMomenta(heatbath &bath, array<double,n> k, const double DT) {
    double s = 0;
    for (int i = 1; i < k.size(); ++i) {
        s = k[i] * (bath.q[0]-bath.q[i])*DT;
        //printf("s=%e \n", s);
        bath.p[0] -= s;
       // printf("updated P = %e \n", bath.p[0]);
        bath.p[i] += s;
    }
}// have to be of same length

template<size_t n>
void updatePositions(heatbath &bath, array<double,n> invM, const double DT) {
    for (int i = 0; i < invM.size() ; ++i) {
       bath.q[i] += bath.p[i]*invM[i]*DT;
    }
}

template<size_t n>
void makeTimestep(heatbath &bath, array<double,n> k, array<double,n> invM, const double DT) {
    updateMomenta(bath,k,DT,n); //update momenta first for a symplectic Euler algorithm
    updatePositions(bath,invM,DT,n);

}

template<size_t n>
void solveEOM(heatbath &bath, array<double,n> invM, array<double,n> k, const double TSPAN[], const double DT ) {
    int nTimesteps = ceil((TSPAN[1]-TSPAN[0])/DT);
    double initialEnergy = H(bath,k,invM);
    bath.initialEnergy = initialEnergy;
    for (int i = 0; i < nTimesteps ; ++i) {
        makeTimestep(bath,k,invM,DT);
      //  bath.trajectory.push_back(bath.q[0]); //  save most recent position

    }

}

double energyError(heatbath &bath, array<double> k, array<double> invM){
 return (H(bath,k,invM)-bath.initialEnergy)/bath.initialEnergy;
}

double momentumError(heatbath &bath) {
    return bath.initialMomentum-sum(Bath.p,Bath.size);
}

//////////////////////////////////////////////////////////
void write_csv(std::string filename, std::string colname, std::array<double> vals){
    std::ofstream myFile(filename);
    myFile << colname << "\n";

    // Send data to the stream
    for(int i = 0; i < vals.size(); ++i)
    {
        myFile << vals.at(i) << "\n";
    }

    myFile.close();
}


#endif
