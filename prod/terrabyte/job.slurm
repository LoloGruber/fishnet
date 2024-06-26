#!/bin/bash
#SBATCH --job-name=settDelAna
#SBATCH --output=stdout.log
#SBATCH --error=stderr.log
#SBATCH --ntasks=1
#SBATCH --time=10:00:00
#SBATCH --partition=hpda2_compute
#SBATCH --cpus-per-task=16
#SBATCH --mem=75gb
#SBATCH --mail-type=end
#SBATCH --mail-user=lorenz.gruber@uni-wuerzburg.de

./settlementDelineation.sh