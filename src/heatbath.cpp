#include <iostream>
#include <cmath>
#include <array>
#include <random>
#include <ctime>
#include <cstring>
#include <fstream>
#include <algorithm>
#include "functions.h"
using std::array;

const int N = 10000; //# of harmonic oscillators in our heatbath
const int NTOTAL = N + 1; // adding the distinguished particle
const double DT =1*pow(10,-6);
const double TSPAN[2] = {0, 1*pow(10,0)};//{0.0,500*DT};
const long long NTIMESTEPS = ceil((TSPAN[1]-TSPAN[0])/DT);
const double GAMMA = 1.2; // expected superdiffusion exponent
const double BETA = 1.0;//1*pow(10,0); // 1/(kB*T)
//const int NSAVE = (int) fmin(pow(10,4),NTIMESTEPS); // max outfile size capped at about 10 MB
const int NSAVE = pow(10,6); // max outfile size capped at about 10 MB

//initialize static heatbath members
double Heatbath::k[NTOTAL] = {0};
double Heatbath::invM[NTOTAL] = {0};
double Heatbath::q[NTOTAL] = {0};
double Heatbath::p[NTOTAL] = {0};
double Heatbath::trajectory[NSAVE] = {0};
double Heatbath::energyErr[NSAVE] = {0};
double Heatbath::momentumErr[NSAVE] = {0};
int Heatbath::size = NTOTAL;
int Heatbath::nSave = NSAVE;
double Heatbath::initialEnergy;
double Heatbath::initialMomentum;


int main() {
    double oscMass = pow(10,2); //mass of heaviest bath oscillator
    double M = 1*pow(10,0); // mass of distinguished particle
    double omegaMin=pow(N,-0.8323), omegaMax=omegaMin*pow(N,1.05764); //highest and lowest eigenfrequency of the bath

    double omega[N];
    double masses[NTOTAL];

    //setting bath parameters
    setEigenfrequencies(omega,omegaMin,omegaMax);
    //setRandomEigenfrequencies(omega,omegaMin,omegaMax);
    computeMasses(masses,oscMass,M,omega,omegaMin,GAMMA);
    computeSpringConstants(Heatbath::k, masses, omega);
    invertMasses(Heatbath::invM,masses);

    Heatbath bath;


//    Normaldev normaldev;

    try {
    generateInitialConditions(bath, M, masses, BETA);
    } catch (const char* msg) {
        std::cerr << msg << std::endl;
    }

    write_initialPositions("../initialConditions/initialPositions.csv","initialPositions" , bath);
    write_initialMomenta("../initialConditions/initialMomenta.csv","initialMomenta" , bath);

    time_t begin,end; // save time it takes to solve EOM
    time(&begin);

    try {
        solveEOM(bath, DT, NTIMESTEPS);
    } catch (const char* msg) {
        std::cerr << msg << std::endl;
    }
    time(&end);

    //generate a random name for outputfiles, write to logfile
    std::random_device rd;
    std::uniform_int_distribution<int> dist(0, 999999);
    int bathName = dist(rd);

    //save trajectory of distinguished particle to file
    write_csv("../csvData/trajectory" + std::to_string(bathName) + ".csv","trajectory" , bath);
    //save errors and runtime to logfile
    std::string filename("./data/log/logfile.txt");
    write_logfile(filename, bathName, begin, end, bath);
    double DTS = TSPAN[1]/ ((double) bath.nSave);
    write_parameters("./data/parameters.csv",N,GAMMA,DTS,TSPAN[1]);

    //write initial conditions to file for checking consistency with python implementation

    //printf("CoM drift = %e",bath.momentumErr[bath.nSave]*TSPAN[1]/sum(masses,NTOTAL));
    printArray_(bath.trajectory,bath.nSave);

    return 0;
}
