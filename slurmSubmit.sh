#!/bin/bash
#SBATCH --job-name=serial_job_test    # Job name
#SBATCH --mail-type=START,END,FAIL          # Mail events (NONE, BEGIN, END, FAIL, ALL)
#SBATCH --mail-user=ledwon@physik.hu-berlin.de	     # Where to send mail
#SBATCH --ntasks=30                   # Run on a single CPU
#SBATCH --mem=1gb                     # Job memory request
#SBATCH --time=30:00:00               # Time limit hrs:min:sec
#SBATCH --output=serial_test_%j.log   # Standard output and error log
pwd; hostname; date

for i in {1..30}
do
   srun --exclude=dong44,dong109,gdong[1-8] ./bin/heatbath &
 #  srun ./bin/heatbath &
done

