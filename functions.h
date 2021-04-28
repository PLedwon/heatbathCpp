#ifndef functions
#define functions
#include <cmath>
#include <random>
#include <vector>
using std::vector;

struct heatbath
{
    vector<double> q;
    vector<double> p;
};

void printVector_(vector<double> a) {
    int i;
    //for (i = 0; i < a.size(); i++) std::cout << a[i] << ' ';
    for (i = 0; i < a.size(); i++) printf("%e ", a[i]);
    printf("\n");
}

double avg(vector<double> a) {
    double average=std::accumulate(a.begin(),a.end(),0.0);
    return (double) average/a.size();
}

vector<double> computeMasses(double oscMass,double M, vector<double>  omega, double omegaMin,const double GAMMA){
  vector<double> masses(omega.size()+1,0);
  masses.front()=M;
  for (int i = 1; i < masses.size() ; i++) {
    masses[i]=oscMass*pow((omega[i-1]/omegaMin),(GAMMA-3))*exp(-omega[i-1]);
  }
  return masses;

}

vector<double> computeSpringConstants(vector<double> masses, vector<double> omega) {
  //int n = nElems(k);
  vector<double> k(masses.size(),0);
  for (int i = 1; i < k.size(); i++) {
    k[i]=masses[i]*pow(omega[i-1],2);
  }
  return k;
}

double H(vector<double> q , vector<double> p, vector<double> k, vector<double> invM) { // compute total energy of the system
  double E = p[0] * p[0] * invM[0];
  for (int i = 1; i < q.size() ; ++i) {
    E += p[i] * p[i] * invM[i] + k[i] * pow(q[i] - q[0], 2);
    //printf("accumulated energy %e \n", E);
    }
  E *= 0.5;
  return  E;
}

double sum(vector<double> p) {
    double mom = std::accumulate(p.begin(),p.end(),0.0);
    return mom;
}

vector<double> setEigenfrequencies(double omegaMin, double omegaMax, int N) {
    double c;
    vector<double> omega(N,0);
    c = (omegaMax - omegaMin)/(omega.size()-1);
    for(int i = 0; i < omega.size() - 1; ++i) // equidistant distribution of eigenfrequencies of the harmonic oscillators
        omega[i] = omegaMin + i*c;
    omega.back() = omegaMax;
    return omega;
}

vector<double> invertMasses(vector<double> masses) {
    vector<double> invM(masses.size(),0);
    for (int i = 0; i < invM.size() ; ++i) {
       invM[i] = 1/masses[i];
    }
    return invM;
}

heatbath generateInitialConditions(double M, vector<double> masses, vector<double> k,const double BETA) {

    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<> d{0,1};
    double pref=pow(BETA,-0.5);

    vector<double> q0(k.size(),0);
    vector<double> p0(k.size(),0);


    //set the initial conditions for the distinguished particle
    q0[0] = 0;
    p0[0] = pref * pow(M,0.5) * d(gen);

    for (int i = 1; i < q0.size(); ++i) {
       q0[i] = q0[0] + pref*pow(k[i],-0.5)*d(gen);
       p0[i] = pref*pow(masses[i],0.5)* d(gen);
    }
    //initialize heatbath with vanishing center of mass velocity
    double avgMomentum = avg(p0);
    double p0sum = sum(p0);
    for (int i = 0; i < p0.size(); ++i) {
        p0[i] -= avgMomentum;
    }
    p0sum = sum(p0);

    //check that total momentum is close to zero
    if (std::abs(p0sum) > pow(10,-14)) {
        throw "Error: CoM velocity is not 0 while initializing heatbath";
    }

    struct heatbath bath = {.q=q0, .p=p0};

    return bath;
}

void updateMomenta(heatbath bath, vector<double> k,const double DT,const int N) {
    double s;
    for (int i = 1; i < N+1; ++i) {
        s = k[i] * (bath.q[0]-bath.q[i])*DT;
        //printf("s=%e \n", s);
        bath.p[0] -= s;
       // printf("updated P = %e \n", bath.p[0]);
        bath.p[i] += s;
    }
}// have to be of same length

void updatePositions(heatbath bath, const vector<double> invM, const double DT, const int N) {
    printVector_(invM);
    for (int i = 0; i < N+1 ; ++i) {
       bath.q[i] += bath.p[i]*invM[i]*DT;
    }
}

void makeTimestep(heatbath bath, vector<double> k, vector<double> invM,const double DT,const int N) {
    updateMomenta(bath,k,DT,N); //update momenta first for a symplectic Euler algorithm
    updatePositions(bath,invM,DT,N);
}

void solveEOM(heatbath bath, vector<double> invM, vector<double> k, const double TSPAN[], const double DT, const int N) {
    int nTimesteps = ceil((TSPAN[1]-TSPAN[0])/DT);
    double initialEnergy;
    for (int i = 0; i < nTimesteps ; ++i) {
        makeTimestep(bath,k,invM,DT,N);

    }

}

//////////////////////////////////////////////////////////


#endif
