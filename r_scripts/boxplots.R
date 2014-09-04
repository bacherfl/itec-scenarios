setwd("/home/dposch/ndnSIM/itec-scenarios/tmp/")


#read CSV files

subfolds<-c("DASH_20M_30RUNS.csv", "DASH_25M_30RUNS.csv", "DASH_30M_30RUNS.csv",
"INA_20M_30RUNS.csv", "INA_25M_30RUNS.csv", "INA_30M_30RUNS.csv")

for(subfold in subfolds) {
	data<-read.csv(subfold)
	# sort by NumConcurStreamers
	sorted<-sort(data$NumConcurStreamers,index.return=TRUE)
	numStreamers<-data$NumConcurStreamers[sorted$ix]
	AVGLevel<-data$AVGLevel[sorted$ix]
	AVGQualitySwitches<-data$AVGQualitySwitches[sorted$ix]
	AVGUnsmooth<-data$AVGUnsmooth[sorted$ix]
	# boxplot(AVGUnsmooth~numStreamers,xlab="Number of cocur. streamers with the same video",ylab="Stalling Seconds", main=subfold,ylim=c(0,260))
	assign(paste(subfold,".AVGLevel",sep=""),AVGLevel)
	assign(paste(subfold,".AVGUnsmooth",sep=""),AVGUnsmooth)
	assign(paste(subfold,".numStreamers",sep=""),numStreamers)
}

#write stuff in lists

avgLevelList<-list(
DASH_20M_30RUNS.csv.AVGLevel[DASH_20M_30RUNS.csv.numStreamers==1], 
DASH_20M_30RUNS.csv.AVGLevel[DASH_20M_30RUNS.csv.numStreamers==2], 
DASH_20M_30RUNS.csv.AVGLevel[DASH_20M_30RUNS.csv.numStreamers==4], 
DASH_20M_30RUNS.csv.AVGLevel[DASH_20M_30RUNS.csv.numStreamers==8], 
DASH_20M_30RUNS.csv.AVGLevel[DASH_20M_30RUNS.csv.numStreamers==16], 
DASH_20M_30RUNS.csv.AVGLevel[DASH_20M_30RUNS.csv.numStreamers>=32],

DASH_25M_30RUNS.csv.AVGLevel[DASH_25M_30RUNS.csv.numStreamers==1], 
DASH_25M_30RUNS.csv.AVGLevel[DASH_25M_30RUNS.csv.numStreamers==2], 
DASH_25M_30RUNS.csv.AVGLevel[DASH_25M_30RUNS.csv.numStreamers==4], 
DASH_25M_30RUNS.csv.AVGLevel[DASH_25M_30RUNS.csv.numStreamers==8], 
DASH_25M_30RUNS.csv.AVGLevel[DASH_25M_30RUNS.csv.numStreamers==16], 
DASH_25M_30RUNS.csv.AVGLevel[DASH_25M_30RUNS.csv.numStreamers>=32],

DASH_30M_30RUNS.csv.AVGLevel[DASH_30M_30RUNS.csv.numStreamers==1], 
DASH_30M_30RUNS.csv.AVGLevel[DASH_30M_30RUNS.csv.numStreamers==2], 
DASH_30M_30RUNS.csv.AVGLevel[DASH_30M_30RUNS.csv.numStreamers==4], 
DASH_30M_30RUNS.csv.AVGLevel[DASH_30M_30RUNS.csv.numStreamers==8], 
DASH_30M_30RUNS.csv.AVGLevel[DASH_30M_30RUNS.csv.numStreamers==16], 
DASH_30M_30RUNS.csv.AVGLevel[DASH_30M_30RUNS.csv.numStreamers>=32]#,
)


AVGUnsmoothList<-list(
DASH_20M_30RUNS.csv.AVGUnsmooth[DASH_20M_30RUNS.csv.numStreamers==1], 
DASH_20M_30RUNS.csv.AVGUnsmooth[DASH_20M_30RUNS.csv.numStreamers==2], 
DASH_20M_30RUNS.csv.AVGUnsmooth[DASH_20M_30RUNS.csv.numStreamers==4], 
DASH_20M_30RUNS.csv.AVGUnsmooth[DASH_20M_30RUNS.csv.numStreamers==8], 
DASH_20M_30RUNS.csv.AVGUnsmooth[DASH_20M_30RUNS.csv.numStreamers==16], 
DASH_20M_30RUNS.csv.AVGUnsmooth[DASH_20M_30RUNS.csv.numStreamers>=32],

DASH_25M_30RUNS.csv.AVGUnsmooth[DASH_25M_30RUNS.csv.numStreamers==1], 
DASH_25M_30RUNS.csv.AVGUnsmooth[DASH_25M_30RUNS.csv.numStreamers==2], 
DASH_25M_30RUNS.csv.AVGUnsmooth[DASH_25M_30RUNS.csv.numStreamers==4], 
DASH_25M_30RUNS.csv.AVGUnsmooth[DASH_25M_30RUNS.csv.numStreamers==8], 
DASH_25M_30RUNS.csv.AVGUnsmooth[DASH_25M_30RUNS.csv.numStreamers==16], 
DASH_25M_30RUNS.csv.AVGUnsmooth[DASH_25M_30RUNS.csv.numStreamers>=32],

DASH_30M_30RUNS.csv.AVGUnsmooth[DASH_30M_30RUNS.csv.numStreamers==1], 
DASH_30M_30RUNS.csv.AVGUnsmooth[DASH_30M_30RUNS.csv.numStreamers==2], 
DASH_30M_30RUNS.csv.AVGUnsmooth[DASH_30M_30RUNS.csv.numStreamers==4], 
DASH_30M_30RUNS.csv.AVGUnsmooth[DASH_30M_30RUNS.csv.numStreamers==8], 
DASH_30M_30RUNS.csv.AVGUnsmooth[DASH_30M_30RUNS.csv.numStreamers==16], 
DASH_30M_30RUNS.csv.AVGUnsmooth[DASH_30M_30RUNS.csv.numStreamers>=32]#,

)








png(filename="dash.png",1920,800)
#pdf("dash.pdf",1280,365 )
# plot DASH

#x11(width=14, height=4)

par(mfrow=c(2,1),
oma = c(4,0,0,0),
mar = c(0,5,0.8,5),
omi = c(1,0,0,0),
cex=0.8
)

boxplot(avgLevelList,ylim=c(0,5),at=c(0,1,2,3,4,5, 7,8,9,10,11,12, 14,15,16,17,18,19),
ylab="",
#xlab="Number of concurrent users streaming the same video",
#names=c("1", "2", "4", "8", "16", expression("" >= "32"), "1", "2", "4", "8", "16", expression("" >= "32"), "1", "2", "4", "8", "16", expression("" >= "32")),
names=rep("",18),
#xaxt="r",
yaxt="n",
boxwex=0.90,
las=1,
staplewex=0.70,
medcol="red",range=1.5)

#for(i in seq(1,length(avgLevelList))) {
#	abline(h=median(avgLevelList[[i]]))
#}



axis(2,0:5,col="black",lwd=1,
labels=c("0", "1", "2", "3", "4", "5"),cex=0.8,
las=1)


mtext("Avg. Layer", side=2,line=3,cex=0.8)



axis(4,0:5,col="black",lwd=1,
labels=c("37.83","38.31","38.74","39.14","39.45","39.65"),cex=0.8,
las=1)

mtext("Y-PSNR [dB]", side=4,line=4,cex=0.8)



boxplot(AVGUnsmoothList,ylim=c(0,250),at=c(0,1,2,3,4,5, 7,8,9,10,11,12, 14,15,16,17,18,19),
ylab="",
names=c("1", "2", "4", "8", "16", expression("" >= "32"), "1", "2", "4", "8", "16", expression("" >= "32"), "1", "2", "4", "8", "16", expression("" >= "32")),
boxwex=0.90,
staplewex=0.70,
las=1,
medcol="red",range=1.5)

mtext("Stalling Time [s]", side=2,line=3,cex=0.8)



#for(i in seq(1,length(AVGUnsmoothList))) {
#	abline(h=median(AVGUnsmoothList[[i]]))
#}

mtext("20 Mbit/s", line = 2,side=1,at=2.5,cex=0.85)
mtext("25 Mbit/s", line = 2,side=1,at=9.5,cex=0.85)
mtext("30 Mbit/s", line = 2,side=1,at=16.5,cex=0.85)


#title(xlab="                20 Mbit/s                                                                   25 MBit/s                                                                   30 Mbit/s",outer=TRUE,line=2)
title(xlab="Number of concurrent clients streaming the same video",outer=TRUE,line=3,cex=1)


dev.off()


#### evaluate INA





avgLevelListINA<-list(
INA_20M_30RUNS.csv.AVGLevel[INA_20M_30RUNS.csv.numStreamers==1], 
INA_20M_30RUNS.csv.AVGLevel[INA_20M_30RUNS.csv.numStreamers==2], 
INA_20M_30RUNS.csv.AVGLevel[INA_20M_30RUNS.csv.numStreamers==4], 
INA_20M_30RUNS.csv.AVGLevel[INA_20M_30RUNS.csv.numStreamers==8], 
INA_20M_30RUNS.csv.AVGLevel[INA_20M_30RUNS.csv.numStreamers==16], 
INA_20M_30RUNS.csv.AVGLevel[INA_20M_30RUNS.csv.numStreamers>=32],

INA_25M_30RUNS.csv.AVGLevel[INA_25M_30RUNS.csv.numStreamers==1], 
INA_25M_30RUNS.csv.AVGLevel[INA_25M_30RUNS.csv.numStreamers==2], 
INA_25M_30RUNS.csv.AVGLevel[INA_25M_30RUNS.csv.numStreamers==4], 
INA_25M_30RUNS.csv.AVGLevel[INA_25M_30RUNS.csv.numStreamers==8], 
INA_25M_30RUNS.csv.AVGLevel[INA_25M_30RUNS.csv.numStreamers==16], 
INA_25M_30RUNS.csv.AVGLevel[INA_25M_30RUNS.csv.numStreamers>=32],

INA_30M_30RUNS.csv.AVGLevel[INA_30M_30RUNS.csv.numStreamers==1], 
INA_30M_30RUNS.csv.AVGLevel[INA_30M_30RUNS.csv.numStreamers==2], 
INA_30M_30RUNS.csv.AVGLevel[INA_30M_30RUNS.csv.numStreamers==4], 
INA_30M_30RUNS.csv.AVGLevel[INA_30M_30RUNS.csv.numStreamers==8], 
INA_30M_30RUNS.csv.AVGLevel[INA_30M_30RUNS.csv.numStreamers==16], 
INA_30M_30RUNS.csv.AVGLevel[INA_30M_30RUNS.csv.numStreamers>=32]#,
)


AVGUnsmoothList<-list(
INA_20M_30RUNS.csv.AVGUnsmooth[INA_20M_30RUNS.csv.numStreamers==1], 
INA_20M_30RUNS.csv.AVGUnsmooth[INA_20M_30RUNS.csv.numStreamers==2], 
INA_20M_30RUNS.csv.AVGUnsmooth[INA_20M_30RUNS.csv.numStreamers==4], 
INA_20M_30RUNS.csv.AVGUnsmooth[INA_20M_30RUNS.csv.numStreamers==8], 
INA_20M_30RUNS.csv.AVGUnsmooth[INA_20M_30RUNS.csv.numStreamers==16], 
INA_20M_30RUNS.csv.AVGUnsmooth[INA_20M_30RUNS.csv.numStreamers>=32],

INA_25M_30RUNS.csv.AVGUnsmooth[INA_25M_30RUNS.csv.numStreamers==1], 
INA_25M_30RUNS.csv.AVGUnsmooth[INA_25M_30RUNS.csv.numStreamers==2], 
INA_25M_30RUNS.csv.AVGUnsmooth[INA_25M_30RUNS.csv.numStreamers==4], 
INA_25M_30RUNS.csv.AVGUnsmooth[INA_25M_30RUNS.csv.numStreamers==8], 
INA_25M_30RUNS.csv.AVGUnsmooth[INA_25M_30RUNS.csv.numStreamers==16], 
INA_25M_30RUNS.csv.AVGUnsmooth[INA_25M_30RUNS.csv.numStreamers>=32],

INA_30M_30RUNS.csv.AVGUnsmooth[INA_30M_30RUNS.csv.numStreamers==1], 
INA_30M_30RUNS.csv.AVGUnsmooth[INA_30M_30RUNS.csv.numStreamers==2], 
INA_30M_30RUNS.csv.AVGUnsmooth[INA_30M_30RUNS.csv.numStreamers==4], 
INA_30M_30RUNS.csv.AVGUnsmooth[INA_30M_30RUNS.csv.numStreamers==8], 
INA_30M_30RUNS.csv.AVGUnsmooth[INA_30M_30RUNS.csv.numStreamers==16], 
INA_30M_30RUNS.csv.AVGUnsmooth[INA_30M_30RUNS.csv.numStreamers>=32]#,

)



#plot INA
png(filename="ina.png",1920,800)

#x11(width=14, height=4)
par(mfrow=c(2,1),
oma = c(4,0,0,0),
mar = c(0,5,0.8,5),
omi = c(1,0,0,0),
cex=0.8
)


boxplot(avgLevelListINA,ylim=c(0,5),at=c(0,1,2,3,4,5, 7,8,9,10,11,12, 14,15,16,17,18,19),
#boxplot(avgLevelListINA,ylim=c(0,5),at=c(0,1,2,3,4,5, 7,8,9,10,11,12, 14,15,16,17,18,19, 21,22,23,24,25,26),
ylab="",
#xlab="Number of concurrent users streaming the same video",
#names=c("1", "2", "4", "8", "16", expression("" >= "32"), "1", "2", "4", "8", "16", expression("" >= "32"), "1", "2", "4", "8", "16", expression("" >= "32")),
names=rep("",18),
#xaxt="r",
boxwex=0.90,
las=1,
yaxt="n",
staplewex=0.70,
medcol="red",range=1.5)

#for(i in seq(1,length(avgLevelListINA))) {
#	abline(h=median(avgLevelListINA[[i]]))
#}

axis(2,0:5,col="black",lwd=1,
labels=c("0", "1", "2", "3", "4", "5"),cex=0.8,
las=1)


mtext("Avg. Layer", side=2,line=3,cex=0.8)


axis(4,0:5,col="black",lwd=1,
labels=c("37.83","38.31","38.74","39.14","39.45","39.65"),cex=0.8,
las=1)



mtext("Y-PSNR [dB]", side=4,line=4,cex=0.8)



boxplot(AVGUnsmoothList,ylim=c(0,250),at=c(0,1,2,3,4,5, 7,8,9,10,11,12, 14,15,16,17,18,19),
ylab="",
names=c("1", "2", "4", "8", "16", expression("" >= "32"), "1", "2", "4", "8", "16", expression("" >= "32"), "1", "2", "4", "8", "16", expression("" >= "32")),
boxwex=0.90,
las=1,
staplewex=0.70,
medcol="red",range=1.5)

mtext("Stalling Time [s]", side=2,line=3,cex=0.8)



#for(i in seq(1,length(AVGUnsmoothList))) {
#	abline(h=median(AVGUnsmoothList[[i]]))
#}


mtext("20 Mbit/s", line = 2,side=1,at=2.5,cex=0.85)
mtext("25 Mbit/s", line = 2,side=1,at=9.5,cex=0.85)
mtext("30 Mbit/s", line = 2,side=1,at=16.5,cex=0.85)


#title(xlab="                20 Mbit/s                                                                   25 MBit/s                                                                   30 Mbit/s",outer=TRUE,line=2)
title(xlab="Number of concurrent clients streaming the same video",outer=TRUE,line=3,cex=1)


dev.off()

avgLevelMedian<-lapply(avgLevelList,median)
avgLevelINAMedian<-lapply(avgLevelListINA,median)



#calculate PSNR for the 1rst entry (clients = 1)
avgLevelMedian[[1]]
PSNR<-38.31+(38.74-38.31)* (avgLevelMedian[[1]]-floor(avgLevelMedian[[1]]))
PSNR

avgLevelINAMedian[[1]]
PSNRINA<-37.83+(38.31-37.83)* (avgLevelINAMedian[[1]]-floor(avgLevelINAMedian[[1]]))
PSNRINA

PSNR-PSNRINA



#calculate PSNR for the 2nd entry (clients = 2)
avgLevelMedian[[2]]
PSNR<-38.31+(38.74-38.31)* (avgLevelMedian[[2]]-floor(avgLevelMedian[[2]]))
PSNR

avgLevelINAMedian[[2]]
PSNRINA<-37.83+(38.31-37.83)* (avgLevelINAMedian[[2]]-floor(avgLevelINAMedian[[2]]))
PSNRINA

PSNR-PSNRINA





#calculate PSNR for the 3rd entry (clients = 4)
avgLevelMedian[[3]]
PSNR<-38.31+(38.74-38.31)* (avgLevelMedian[[3]]-floor(avgLevelMedian[[3]]))
PSNR

avgLevelINAMedian[[3]]
PSNRINA<-38.31+(38.74-38.31)* (avgLevelINAMedian[[3]]-floor(avgLevelINAMedian[[3]]))
PSNRINA

PSNR-PSNRINA





#calculate PSNR for the 4th entry (clients = 8)
avgLevelMedian[[4]]
PSNR<-39.14+(39.45-39.14)* (avgLevelMedian[[4]]-floor(avgLevelMedian[[4]]))
PSNR

avgLevelINAMedian[[4]]
PSNRINA<-38.31+(38.74-38.31)* (avgLevelINAMedian[[4]]-floor(avgLevelINAMedian[[4]]))
PSNRINA

PSNR-PSNRINA





#calculate PSNR for the 5th entry (clients = 16)
avgLevelMedian[[5]]
PSNR<-39.45+(39.65-39.45)* (avgLevelMedian[[5]]-floor(avgLevelMedian[[5]]))
PSNR

avgLevelINAMedian[[5]]
PSNRINA<-38.74+(39.14-38.74)* (avgLevelINAMedian[[5]]-floor(avgLevelINAMedian[[5]]))
PSNRINA

PSNR-PSNRINA



#calculate PSNR for the 6th entry (clients >= 32)
avgLevelMedian[[6]]
PSNR<-39.45+(39.65-39.45)* (avgLevelMedian[[6]]-floor(avgLevelMedian[[6]]))
PSNR

avgLevelINAMedian[[6]]
PSNRINA<-38.74+(39.14-38.74)* (avgLevelINAMedian[[6]]-floor(avgLevelINAMedian[[6]]))
PSNRINA

PSNR-PSNRINA



# 25 mbit run

#calculate PSNR for the 1rst entry (clients = 1)
avgLevelMedian[[7]]
PSNR<-38.31+(38.74-38.31)* (avgLevelMedian[[7]]-floor(avgLevelMedian[[7]]))
PSNR

avgLevelINAMedian[[7]]
PSNRINA<-37.83+(38.31-37.83)* (avgLevelINAMedian[[7]]-floor(avgLevelINAMedian[[7]]))
PSNRINA

PSNR-PSNRINA



#calculate PSNR for the 2nd entry (clients = 2)
avgLevelMedian[[8]]
PSNR<-38.31+(38.74-38.31)* (avgLevelMedian[[8]]-floor(avgLevelMedian[[8]]))
PSNR

avgLevelINAMedian[[8]]
PSNRINA<-38.31+(38.74-38.31)*  (avgLevelINAMedian[[8]]-floor(avgLevelINAMedian[[8]]))
PSNRINA

PSNR-PSNRINA



#calculate PSNR for  (clients = 16)
avgLevelMedian[[11]]
PSNR<-39.45+(39.65-39.45)* (avgLevelMedian[[11]]-floor(avgLevelMedian[[11]]))
PSNR

avgLevelINAMedian[[11]]
PSNRINA<-39.14+(39.14-38.74)*  (avgLevelINAMedian[[11]]-floor(avgLevelINAMedian[[11]]))
PSNRINA

PSNR-PSNRINA





#calculate PSNR for  (clients = 32)
avgLevelMedian[[12]]
PSNR<-39.45+(39.65-39.45)* (avgLevelMedian[[12]]-floor(avgLevelMedian[[12]]))
PSNR

avgLevelINAMedian[[12]]
PSNRINA<-39.14+(39.14-38.74)*  (avgLevelINAMedian[[12]]-floor(avgLevelINAMedian[[12]]))
PSNRINA

PSNR-PSNRINA





#calculate PSNR for  (clients = 16)
avgLevelMedian[[17]]
PSNR<-39.45+(39.65-39.45)* (avgLevelMedian[[17]]-floor(avgLevelMedian[[17]]))
PSNR

avgLevelINAMedian[[17]]
PSNRINA<-39.45+(39.45-39.14)*  (avgLevelINAMedian[[17]]-floor(avgLevelINAMedian[[17]]))
PSNRINA

PSNR-PSNRINA



#calculate PSNR for the 2nd entry (clients = 2)
avgLevelMedian[[18]]
PSNR<-39.45+(39.65-39.45)* (avgLevelMedian[[18]]-floor(avgLevelMedian[[18]]))
PSNR

avgLevelINAMedian[[18]]
PSNRINA<-39.14+(39.14-38.74)*  (avgLevelINAMedian[[18]]-floor(avgLevelINAMedian[[18]]))
PSNRINA

PSNR-PSNRINA


