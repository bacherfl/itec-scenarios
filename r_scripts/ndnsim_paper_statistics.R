# adapt working directory
setwd("Z:\\concert\\ndnSIM_paper_simulations")
num_runs<-30
num_clients<-100
num_segments<-299

# the script will create variables called the same as the folder that contains the simulation runs




# loop through all variations of the simulations
subfolds<-dir()

for(subfold in subfolds) {
	print(subfold)
	
	# collect stats
	all_levels=c()
	all_unsmooth=c()
	all_unsmooth_sum=c()
	all_avgClientLevel=c()
	all_buffer=c()

	# loop over all runs in that subfolder
	for(run in seq(0,num_runs-1)) {
		data_folder = paste(subfold,"/output_run",sep="")
		data_folder = paste(data_folder, toString(run),sep="")
		data_folder = paste(data_folder, "/", sep="")
		
		# loop over all clients
		for(client in seq(0,num_clients-1)) {
			filename = paste(data_folder, "ContentDst",sep="")
			filename = paste(filename, toString(client), sep="")
			filename = paste(filename, ".txt", sep="")
		
			data<-read.csv(filename)
			segmentNr<-as.vector(data$SegmentNr[0:num_segments])
			levels<-as.vector(data$Level[0:num_segments])
			buffer<-as.vector(data$Buffer[0:num_segments])
			unsmooth<-as.vector(data$Unsmooth[0:num_segments])
			requested<-as.vector(data$Requested[0:num_segments])
			goodput<-as.vector(data$Goodput[0:num_segments])

			# sum over unsmooth seconds for this client
			sum_unsmooth<-sum(unsmooth)	
			all_unsmooth_sum<-c(all_unsmooth_sum,sum_unsmooth)


			avg_quality<-mean(levels)
			all_avgClientLevel<-c(all_avgClientLevel,avg_quality)


			# fill in vector
			all_levels=c(all_levels,levels)
			all_unsmooth=c(all_unsmooth,unsmooth)
			all_buffer=c(all_buffer,buffer)				
		}
	}


	d<-data.frame("Levels"=all_levels,Stalls=all_unsmooth,Buffer=all_buffer)
	assign(subfold,d)
	assign(paste(subfold,".AvgClientLevel",sep=""),all_avgClientLevel)
	assign(paste(subfold,".UnsmoothSum",sep=""),all_unsmooth_sum)
	
}



##################
# compare buffer #
##################
# compare variances for Buffer, we see that they are not equal
var.test(dash_svc_bottleneck_20mbps$Buffer,dash_svc_bottleneck_30mbps$Buffer)
var.test(dash_svc_bottleneck_30mbps$Buffer,dash_svc_bottleneck_40mbps$Buffer)
# therefore we set var.equal=FALSE in t.test


# test: H0: 20Mbit$Buffer > 30Mbit$Buffer
t.test(dash_svc_bottleneck_20mbps$Buffer,dash_svc_bottleneck_30mbps$Buffer,alternative="less",paired=TRUE,var.equal=FALSE)
# test: H0: 30Mbit$Buffer > 40Mbit$Buffer
t.test(dash_svc_bottleneck_30mbps$Buffer,dash_svc_bottleneck_40mbps$Buffer,alternative="less",paired=TRUE,var.equal=FALSE)


##################
# compare Levels #
##################
# compare variances for Levels, we see that they are not equal
var.test(dash_svc_bottleneck_20mbps$Levels,dash_svc_bottleneck_30mbps$Levels)
var.test(dash_svc_bottleneck_30mbps$Levels,dash_svc_bottleneck_40mbps$Levels)
# therefore we set var.equal=FALSE in t.test

# compare means of Levels
t.test(dash_svc_bottleneck_20mbps$Levels,dash_svc_bottleneck_30mbps$Levels,alternative="less",paired=TRUE,var.equal=FALSE)
t.test(dash_svc_bottleneck_30mbps$Levels,dash_svc_bottleneck_40mbps$Levels,alternative="less",paired=TRUE,var.equal=FALSE)



#######################
# compare Sum(Stalls) #
#######################

# compare variances for Buffer, we see that they are not equal
var.test(dash_svc_bottleneck_20mbps.UnsmoothSum,dash_svc_bottleneck_30mbps.UnsmoothSum)
var.test(dash_svc_bottleneck_30mbps.UnsmoothSum,dash_svc_bottleneck_40mbps.UnsmoothSum)
# therefore we set var.equal=FALSE in t.test


t.test(dash_svc_bottleneck_20mbps.UnsmoothSum,dash_svc_bottleneck_30mbps.UnsmoothSum,alternative="greater",paired=TRUE,var.equal=FALSE)
t.test(dash_svc_bottleneck_30mbps.UnsmoothSum,dash_svc_bottleneck_40mbps.UnsmoothSum,alternative="greater",paired=TRUE,var.equal=FALSE)




par(mfrow=c(1,3))
boxplot(dash_svc_bottleneck_20mbps$Levels)
boxplot(dash_svc_bottleneck_30mbps$Levels)
boxplot(dash_svc_bottleneck_40mbps$Levels)





mean_levels<-c(
mean(dash_svc_bottleneck_20mbps$Levels),
mean(dash_svc_bottleneck_30mbps$Levels),
mean(dash_svc_bottleneck_40mbps$Levels)
)


mean_stalls<-c(
mean(dash_svc_bottleneck_20mbps.UnsmoothSum),
mean(dash_svc_bottleneck_20mbps.UnsmoothSum),
mean(dash_svc_bottleneck_20mbps.UnsmoothSum)
)


max_stalls<-c(
max(dash_svc_bottleneck_20mbps.UnsmoothSum),
max(dash_svc_bottleneck_30mbps.UnsmoothSum),
max(dash_svc_bottleneck_40mbps.UnsmoothSum)
)

min_avgLevel<-c(
min(dash_svc_bottleneck_20mbps.AvgClientLevel),
min(dash_svc_bottleneck_30mbps.AvgClientLevel),
min(dash_svc_bottleneck_40mbps.AvgClientLevel)
)

mean_buffer<-c(
mean(dash_svc_bottleneck_20mbps$Buffer),
mean(dash_svc_bottleneck_30mbps$Buffer),
mean(dash_svc_bottleneck_40mbps$Buffer)
)


results<-data.frame(AvgLevels=mean_levels,AvgStalls=mean_stalls,MaxStalls=max_stalls,MinClientLevel=min_avgLevel,AvgBuffer=mean_buffer)
rownames(results)<-c("Dash 20 Mbit/s", "Dash 30 MBit/s", "Dash 40 Mbit/s")

