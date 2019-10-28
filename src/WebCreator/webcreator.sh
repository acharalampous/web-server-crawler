#!/bin/bash
# webcreator.sh #

lnLimit=10000 # page index limit
rootdir=$1 # root_directory that contains sites
docfile=$2 # big document file with text
numOfSites=$3 # num of sites to create
numOfPages=$4 # num of pages that each site will have

: '
echo "***************************"
echo *rootDirectory: $rootdir 
echo *Num of Sites: $numOfSites
echo *docfile: $docfile
echo *Num of Pages: $numOfPages
echo "***************************"
echo -e 
'

# Check if root_dir exists #
if [ ! -d $rootdir ] 
then
	echo Directory given does not exist! Abort.
	exit -1
else
	# if there is a / in rootdir given, remove it #
	rootdir=`echo -e $rootdir | tr -d '/'`
fi


# Check if textfile exists #
if [ ! -f $docfile ]
then
	echo Textfile given does not exist! Abort.
	exit -2
fi


# Check that sites and pages number are integers and greater than 0 #
if [ "$numOfSites" -eq "$numOfSites" ] 2>/dev/null; 
then
	#echo "-->Number of sites given is ok!"
	numOfSites=$((numOfSites))
else
	echo "Number of sites given is not a number! Abort."
	exit -3	
fi
if [ $numOfSites -lt 1 ]
then
	echo "Number of sites must be greater than 0! $numOfSites given! Abort."
	exit -3
fi


if [ "$numOfPages" -eq "$numOfPages" ] 2>/dev/null; 
then
	#echo "-->Number of pages given is ok!"
	numOfPages=$((numOfPages))
else
	echo "Number of pages given is not a number! Abort."
	exit -4
fi
if [ $numOfPages -lt 1 ]
then
	#echo "Number of pages must be greater than 0! $numOfPages given! Abort."
	exit -4
fi

# Get number of lines of textfile(empty lines are included) #
numOfLines=$(wc -l < $docfile)
#echo **Num of Lines: $numOfLines

# Confirm that file has at least 10K lines #
if [ $numOfLines -lt $lnLimit ] 
then
	echo File given has less than 10000 lines! Abort.
	exit -5
fi


# If directory is not empty, purge #
if [ "$(ls -A $rootdir)" ]; then
    echo "Warning: directory is full, purging ..."
	rm -rf $rootdir/*
fi


# An array with all the links will be created. The names #
# of the pages will be generated before they be created  #
# and will be stored in an array, so each time a page    #
# can get the links that will store in its body          #

totalLinks=$((numOfSites * numOfPages))
allLinks=() # all the links(their paths)
used=() # link 0: not used, 1: used in current page
isLink=() # 0: is incoming link, 1: is not


# Create total number of page names #
pageNum=0
while [ $pageNum -lt $totalLinks ]
do
	site=$((pageNum / numOfPages)) # get number of site
	pageID=$((RANDOM % 10000))
	pageName="site$site/page$site"
	pageName+="_$pageID.html" # create full page name
	
	exists=0
	# Check if page with same name already exists #
	for i in $(seq $((site * numOfPages)) $((pageNum - 1)))
	do
		if [ "$pageName" == "${allLinks[i]}" ]
		then
			exists=1
		fi
	done

	# if page does not already exist, save it #
	if [ $exists -eq 0 ]
	then
		allLinks[pageNum]=$pageName
		used[pageNum]=0
		isLink[pageNum]=0
		pageNum=$((pageNum + 1))
	fi
done


# find number of external links #
case $numOfSites in
	1)	q=0;; # there will be no external links
	2)	q=1;; # there will be only one external link
	*)	q=$(((numOfSites / 2) + 1));; # number of external links
esac

f=$(((numOfPages/2) + 1)) # number of internal links 


# fetch text in memory #
text=()
i=0
while read line;
do
	text[i]=$line
	i=$((i+1))
done < $docfile


# Create all sites and pages #
for siteNum in $(seq 0 $((numOfSites - 1)))
do
	siteName="site$siteNum"
	echo " Creating website $siteNum"
	mkdir $rootdir/$siteName # create site folder

	# Create all pages of sites #
	for pageNum in $(seq 0 $((numOfPages - 1)))
	do
		index=$(((siteNum * numOfPages) + pageNum)) # index of current page in array of links 
		
		# set all links as unused
		for i in $(seq 0 $((totalLinks - 1)))
		do
			used[i]=0
		done

		# if numberOfPages given <= 2, we can use link to the page itself #
		if [ $numOfPages -gt 2 ]
		then
			# set itself as used(page cannot be linked to itself) #
			used[$index]=1;
		fi

		# generate k and m parameters #
		k=$((2 + (RANDOM % $((numOfLines - 1998)))))
		m=$((1001 + (RANDOM % 999)))

		newfile="$rootdir/${allLinks[index]}"
		echo -e "  Creating page $newfile with $m lines starting at line $k"
		touch $newfile # create page #

		setLines=$((m / (f + q))) # number of lines per set
		extraLines=$((m % (f + g))) # number of lines that will have one extra line
		remainingf=$f
		remainingq=$q		

		allText="<!DOCTYPE html>\n<html>\n\t<body>"
		
		setIndex=0 # number of set(or number of link to be inserted)
		
		# while there are links to insert #
		while [ $remainingf != 0 -o $remainingq != 0 ]
		do
			linesToInsert=$setLines # lines to print before current link

			if [ $setIndex -lt $extraLines ] 
			then
				linesToInsert=$((linesToInsert + 1)) # the remaining lines will be divided to the first links
			fi
	
			flag=0 # until valid link is found
			typeOfLink=2 # type of link to search for 0: external, 1: internal, 2: both
			if [ $remainingf -eq 0 ] # no more internal links left
			then
				typeOfLink=0
			elif [ $remainingq -eq 0 ] # no more external links left
			then
				typeOfLink=1
			else
				typeOfLink=$((RANDOM % 2))
			fi
			
			while [ $flag -eq 0 ]
			do
				case $typeOfLink in
				# get external link #
				0)	if [ $siteNum -eq 0 ] # first site 
					then
						lim=$(((siteNum + 1) * numOfPages))
						choice=$((lim + (RANDOM % (totalLinks - lim))))	
					elif [ $siteNum -eq $((numOfSites - 1)) ] # last site
					then
						choice=$((RANDOM % (siteNum * numOfPages)))
					else
						#	filters are used to exlude the internal links area #
						#	1 filter will be 1 and the other 0. This will determine #
						#	which area will be chosen(before or after internal) #
						filter1=$((RANDOM % 2)) 
						filter2=$(((filter1 + 1) % 2)) # create the opposite of filter1
						range1=$((RANDOM % (siteNum * numOfPages))) # befoe internal links
						lim=$(((siteNum + 1) * numOfPages))
						range2=$(((lim + (RANDOM % (totalLinks - lim))))) # after internal
					
						choice=$(((filter1 * range1) + (filter2 * range2)))
					fi ;; # end if siteNum ==0,1,...,n
				# get intenal link #
				1)	choice=$(((siteNum * numOfPages) + (RANDOM % numOfPages)));;
				
				esac
				
				# if specific link not already in page, place it #
				if [ ${used[$choice]} -eq 0 ]
				then
					echo -e "    Adding link to $rootdir/${allLinks[$choice]}"
					if [ $typeOfLink -eq 0 ] 
					then
						link="../${allLinks[$choice]}" # create link to external page
						remainingq=$((remainingq - 1))
					else
						link="$( cut -d '/' -f 2 <<< "${allLinks[$choice]}" )" # create link to internal page
						remainingf=$((remainingf - 1))
					fi

					newLink="\n\t\t"
					placedLink=$((RANDOM % linesToInsert)) # number of line it will be placed after
					for num in $(seq 1 $linesToInsert) 
					do
						newLink+=" ${text[$k]}"
						if [ $placedLink -eq $num ]
						then
								newLink+=" <a href=$link>link$setIndex</a>"
						fi
						k=$((k + 1))
					done

					allText+="$newLink"
					used[$choice]=1
					isLink[$choice]=1
					flag=1
				fi
			done # end while flag=0
			setIndex=$((setIndex + 1))
		done # end while there are links
		allText+="\n\t</body>\n</html>" # write closing html tags	
		echo -e "$allText" >> $newfile # write content to file	
	done # end while creating pages
done # end while creating sites 


# Check if every page has incoming links #
allLinked=1
for pageNum in $(seq 0 $((totalLinks - 1)))
do
	if [ ${isLink[pageNum]} -eq 0 ] # found page with no incoming link
	then
		allLinked=0
		break
	fi
done

if [ $allLinked -eq 1 ]
then
	echo "All pages have at least one incoming link"
else
	echo "Not all pages have incoming link"
fi

echo "Done."
