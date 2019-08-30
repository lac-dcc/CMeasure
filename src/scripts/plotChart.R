###############################################################
# Script for generating charts out of power samples collected #
# by the CMeasure tool.                                       #
# Author: Rubens Emilio, Junio Cezar                          #
###############################################################

#  
#  This program is free software: you can redistribute it and/or modify  
#  it under the terms of the GNU General Public License as published by  
#  the Free Software Foundation, version 3.
# 
#  This program is distributed in the hope that it will be useful, but 
#  WITHOUT ANY WARRANTY; without even the implied warranty of 
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
#  General Public License for more details.
# 
#  You should have received a copy of the GNU General Public License 
#  along with this program. If not, see <http://www.gnu.org/licenses/>.
# 

# Usage: RScript plotChart.R inputFile skip
# where inputFile is the file containing the raw file generated
# by CMeasure and skip is the an integer describing the skip
# rate during sample collection.

# Arguments
# commandArgs -> gets arguments from the command line 
# as.numeric -> converts string to numeric value

print("usage: Rscript plotChart.R output_from_cmeasure 50")

args = commandArgs();
inpath <- args[6];
skipLen <- as.numeric(args[7]);

# c -> Input
dbs <- c();
maxX <- 0;
maxY <- 0;

# Input of data 

# Get the file 
filename <- inpath
pdfname <- paste(c(inpath, ".pdf"), collapse="")
	
# Put data on a table
db <- read.table(filename);
names(db) <- c("time", "power");
db <- db[seq(1, length(db$time), skipLen),];

# dbs <- c(dbs, db);
maxX <- max(c(maxX, db$time));
maxY <- max(c(maxY, db$power));

# Set or query graphical parameters 
numTests = 5 # eu fiz
par(mfrow=c(numTests, 1), mar=c(2, 5, 1, 1), oma=c(3, 0, 0, 0));

# Plot graphs 
# Get data from table 
# Labels for each value (X, Y)
pdf(pdfname, width=10, height=3)
par(mar=c(4,4,2,2))

plot(db$time, db$power, pch="",
	xlim=c(0, maxX), ylim=c(0, maxY),
	xlab="Time (s)", ylab="iPower (W)");
lines(db$time, db$power);
