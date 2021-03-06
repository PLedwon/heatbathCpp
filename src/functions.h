#ifndef functions
#define functions
#include <cmath>
#include <iostream>
#include <random>
#include <limits>
#include <utility>
#include <fstream>
#include <array>
#include <tuple>
using std::array;

typedef unsigned long long int Ullong;
typedef double Doub;

class Heatbath {

  public:
    static int size;
    static int nSave;
    static double k[];
    static double invM[];
    static double q[];
    static double p[];
    static double trajectory[];
    static double energyErr[];
    static double momentumErr[];
    static double initialEnergy;
    static double initialMomentum;

};

void printArray_(double a[], int size) {
    int i;
    for (i = 0; i < size; i++) {
        //std::cout << a[i] << ' ';
        printf("%e ", a[i]);
	std::cout << '\n';
    }
    std::cout << '\n';
}

double sum(double p[], int size) {
    double mom = 0;
    for (int i = 0; i < size; ++i) {
        mom += p[i];
    }
    return mom;

}double avg(double a[], int size) {
    return (double) sum(a,size)/size;
}

template<size_t n>
void computeMasses(double (&masses)[n], double oscMass, double M, double (&omega)[n-1], double omegaMin, const double GAMMA){
  masses[0]=M;
  for (int i = 1; i < n ; i++) {
    masses[i]=oscMass*pow((omega[i-1]/omegaMin),(GAMMA-3))*exp(-omega[i-1]);
  }

}

template<size_t n>
void computeSpringConstants(double (&k)[n], double (&masses)[n], double (&omega)[n-1]) {
  k[0] = 0;
  for (int i = 1; i < n; i++) {
    k[i]=masses[i]*pow(omega[i-1],2)*pow(10,0);
  }
}

double H(Heatbath &bath) { // compute total energy of the system
  double E = bath.p[0] * bath.p[0] * bath.invM[0];
  for (int i = 1; i < bath.size ; ++i) {
    E += bath.p[i] * bath.p[i] * bath.invM[i] + bath.k[i] * pow(bath.q[i] - bath.q[0], 2);
    //printf("accumulated energy %e \n", E);
    }
  E *= 0.5;
  return  E;
}


template< size_t n>
void setEigenfrequencies(double (&omega)[n], double omegaMin, double omegaMax) {
    double c;
    c = (omegaMax - omegaMin)/(n-1);
    for(int i = 0; i < n ; ++i) // equidistant distribution of eigenfrequencies of the harmonic oscillators
        omega[i] = omegaMin + i*c;
    //omega.back() = omegaMax;
}

template< size_t n>
void setRandomEigenfrequencies(double (&omega)[n], double omegaMin, double omegaMax) {

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(omegaMin,omegaMax);

    for(int i = 0; i < n ; ++i) // equidistant distribution of eigenfrequencies of the harmonic oscillators
        omega[i] = distribution(generator);
}
template< size_t n>
void invertMasses(double (&invM)[n], double (&masses)[n]) {
    for (int i = 0; i < n; ++i) {
       invM[i] = 1/masses[i];
    }
}


std::pair<double, double> generateGaussianNoise(double mu, double sigma)
{
    constexpr double epsilon = std::numeric_limits<double>::epsilon();
    constexpr double two_pi = 2.0 * M_PI;

    //initialize the random uniform number generator (runif) in a range 0 to 1
    static std::mt19937 rng(std::random_device{}()); // Standard mersenne_twister_engine seeded with rd()
    static std::uniform_real_distribution<> runif(0.0, 1.0);

    //create two random numbers, make sure u1 is greater than epsilon
    double u1, u2;
    do
    {
        u1 = runif(rng);
        u2 = runif(rng);
    }
    while (u1 <= epsilon);

    //compute z0 and z1
    auto mag = sigma * sqrt(-2.0 * log(u1));
    auto z0  = mag * cos(two_pi * u2) + mu;
    auto z1  = mag * sin(two_pi * u2) + mu;

    return std::make_pair(z0, z1);
}

/*
// generate normally distributed random number with ratio of uniforms method, see "Numerical recipes in C"
struct Normaldev : Ran {
            //Structure for normal deviates.
    Doub mu,sig;
    Normaldev(Doub mmu, Doub ssig, Ullong i)
            : Ran(i), mu(mmu), sig(ssig){}
        Doub dev() {
        // Return a normal deviate.
                Doub u,v,x,y,q;
        do {
            u = Doub();
            v = 1.7156*(Doub()-0.5);
            x = u - 0.449871;
            y = fabs(v) + 0.386595;
            q = pow(x,0.5) + y*(0.19600*y-0.25472*x);
        } while (q > 0.27597
                 && (q > 0.27846 || pow(v,0.5) > -4.*log(u)*pow(u,0.5)));
        return mu + sig*v/u;
    }
};
*/


void generateInitialConditions(Heatbath &bath, double M,  double masses[], const double BETA) {


    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<> d{0,1};


    double pref=pow(BETA,-0.5);

    //set the initial conditions for the distinguished particle
    std::pair<double, double> rndPair;
    rndPair = generateGaussianNoise(0.0,1.0);

    bath.q[0] = 0; //
    bath.p[0] = pref * pow(M,0.5) * rndPair.second ;
    //bath.p[0] = pref * pow(M,0.5) * d(gen);

    for (int i = 1; i < bath.size; ++i) {

       rndPair = generateGaussianNoise(0,1);
       bath.q[i] = bath.q[0] + pref*pow(bath.k[i],-0.5) * rndPair.first;//d(gen);
       bath.p[i] = pref*pow(masses[i],0.5) * rndPair.second; //d(gen);
    }
    //initialize heatbath with vanishing center of mass velocity
    double avgMomentum = avg(bath.p,bath.size);
    double psum;
    for (int i = 0; i < bath.size; ++i) {
        bath.p[i] -= avgMomentum;
    }
    psum = sum(bath.p,bath.size);

    //check that total momentum is close to zero
    if (std::abs(psum) > pow(10,-7)) {
        throw "Error: CoM velocity is not 0 while initializing heatbath";
    }
    bath.initialEnergy = H(bath);
    bath.initialMomentum = sum(bath.p,bath.size);
}



void updateMomenta(Heatbath &bath, const double DT) {
    double s = 0;
    for (int i = 1; i < bath.size; ++i) {
        s = bath.k[i] * (bath.q[0]-bath.q[i])*DT;
        //printf("s=%e \n", s);
        bath.p[0] -= s;
       // printf("updated P = %e \n", bath.p[0]);
        bath.p[i] += s;
    }
}// have to be of same length

void updatePositions(Heatbath &bath, const double DT) {
    for (int i = 0; i < bath.size ; ++i) {
       bath.q[i] += bath.p[i]*bath.invM[i]*DT;
    }
}

void makeTimestep(Heatbath &bath, const double DT) {
    updateMomenta(bath, DT); //update momenta first for a symplectic Euler algorithm
    updatePositions(bath, DT);
}

 double energyError(Heatbath &bath){
  return std::abs((H(bath)-bath.initialEnergy)/bath.initialEnergy);
 }

 double momentumError(Heatbath &bath) {
     return std::abs(bath.initialMomentum-sum(bath.p,bath.size));
     //return bath.initialMomentum-sum(bath.p,bath.size);
 }

void write_time(std::string filename) {
    std::ofstream myFile(filename);
    myFile << "10% done" << "\n";
    myFile.close();
}

void solveEOM(Heatbath &bath, const double DT, const long long NTIMESTEPS) {

   bath.trajectory[0]=bath.q[0];

    int saveIndex = NTIMESTEPS/bath.nSave;
    int j=1;
    for (int i = 0; i < NTIMESTEPS ; ++i) {
        makeTimestep(bath, DT);
        if (i % saveIndex == 0 && j < bath.nSave) {
            bath.trajectory[j] = bath.q[0]; //  save most recent position of distinguished particle
            //printf("Q = %e", bath.trajectory[j]);
            //std::cout << '\n';
            bath.energyErr[j] = energyError(bath);
            bath.momentumErr[j] = momentumError(bath);
            //printf("energy error = %e", bath.energyErr[j]);
            //std::cout << '\n';
            /* if (bath.trajectory[0] !=0) {
                 printf("initial position = %e \n",bath.trajectory[0]);
             }
             */
            if (bath.energyErr[j]>pow(10,-4)) {
                printf("energy error = %e", bath.energyErr[j]);
                throw "relative energy error > 10^(-4)";
                std::cout << '\n';
            }
            if (bath.momentumErr[j]>5*pow(10,-7)) {
                printf("momentum error = %e", bath.momentumErr[j]);
                throw "absolute momentum error > 10^(-7)";
                std::cout << '\n';
            }

            if (j==ceil(0.1*bath.nSave)) {
                write_time("./data/log/timelog.txt");
            }
            j++;
        }
    }
}

////////////////////////////////////////////////////////////
void write_csv(std::string filename, std::string colname, Heatbath &bath){
    std::ofstream myFile(filename);
    //myFile << colname << "\n";

    // Send data to the stream
    for(int i = 0; i < bath.nSave; ++i)
    {
        myFile << bath.trajectory[i] << "\n";
    }

    myFile.close();
}

void write_initialPositions(std::string filename, std::string colname, Heatbath &bath){
    std::ofstream myFile(filename);
    //myFile << colname << "\n";

    // Send data to the stream
    for(int i = 0; i < bath.size; ++i)
    {
        myFile << bath.q[i] << "\n";
    }

    myFile.close();
}

void write_initialMomenta(std::string filename, std::string colname, Heatbath &bath){
    std::ofstream myFile(filename);
    //myFile << colname << "\n";

    // Send data to the stream
    for(int i = 0; i < bath.size; ++i)
    {
        myFile << bath.p[i] << "\n";
    }

    myFile.close();
}

void write_parameters(std::string filename, const int N , const double GAMMA , double DTS, const double tspan){
    std::ofstream myFile(filename);
    myFile << "bathsize, gamma, DTS, t" << "\n";
    myFile<< std::scientific << N <<", " << GAMMA  << ", " << DTS << ", " << tspan <<  "\n";
    myFile.close();
}

void write_logfile(std::string filename, int name, time_t begin, time_t end, Heatbath &bath) {
    double maxEnergyErr = *std::max_element(bath.energyErr,bath.energyErr+bath.nSave);
    double maxMomentumErr = *std::max_element(bath.momentumErr,bath.momentumErr+bath.nSave);
    double difference = difftime(end,begin)/3600.0;

    std::fstream file;
    file.open(filename, std::ios_base::app | std::ios_base::in);
    if (file.is_open())
        file.precision(2);
    file << std::scientific << std::to_string(name) << ", " << maxEnergyErr << ", " << maxMomentumErr << ", "
         << avg(bath.energyErr, bath.nSave) << ", " << avg(bath.momentumErr, bath.nSave) << ", " << difference
         << std::endl;
    file.close();
}



#endif
