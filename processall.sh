
EXECUTABLE=Default/DP2.exe

DATABASE_DIR=./NaturalNeighbor/DP2/natural
OUTPUT_DIR=./output

REPORT=shading-`date +%s`.csv
SQL=shadingsql-`date +%s`.sql

export PATH=$PATH:/c/boost/boost_1_44_0/lib:/c/OpenCV2.2/OpenCV-2.2.0-mingw32/bin:/c/OpenCV2.2/OpenCV2.2-vs2010/bin
DEFAULT_PARAMS="--header=false --type=NORMALIZED --K=04 --MC=16 --GD=08 --SL=08 --DIFF_BUC=4 --PERC_BUC=61 --MVP=10"

shopt -s extglob

createtable=0

insert_field()
{
   if [ $createtable -eq 1 ]; then

       if [ $col -eq 0 ]; then
           col=2
           echo "$1 $2" >> "$SQL"
       else
           echo ",$1 $2" >> "$SQL"
       fi

       return 0
   fi

   if [ $col -eq 0 ]; then
       col=2
       echo "$1 = '$3'" >> "$SQL"
   else
       echo ",$1 = '$3'" >> "$SQL"
   fi
}

insert_param()
{
   if [ $createtable -eq 1 ]; then

       if [ $col -eq 0 ]; then
           col=2
           echo "$1 $2" >> "$SQL"
       else
           echo ",$1 $2" >> "$SQL"
       fi

       return 0;
   fi

   local PARAMNAME=$1
   local PARAMVALUE=$3
   local length=${#PARAMNAME}
   local PARAMVALUE=${PARAMVALUE:$length}
   echo ",$PARAMNAME = '$PARAMVALUE'" >> "$SQL"
}

getparams() {

    local default_params=$1
    local changed_param=$2

    local paramprefix=`echo $changed_param | cut -d= -f1`

    PARAMNAME=`echo $paramprefix | cut -d- -f3`
    PARAMVALUE=`echo $changed_param | cut -d= -f2`
    
    params=${default_params/$paramprefix=+([0-9])/}
    params="${params} ${changed_param}"
    return 0

}

execute() {

    echo "--------------------------------------------------------"
    local imgpath=$1
    local params=$2
    local imgfile=${imgpath##*/}
    imgfile=${imgfile%.*}

    local bgvalue=`expr "${imgfile}" : '.*bg'`
    bgvalue=${imgfile:$bgvalue:6}
    if [ "$bgcolor" != "" ]; then
        bgvalue="$bgcolor"
    fi
    if [ "$bgcolor" != "INFER" ]; then
        params="${params} --BG=${bgvalue}"
    fi
    #echo "expr ${imgfile} : '.*bg'"

    local tmpout="out.jpg"

    echo "Processing '${imgpath}' with params '${params}'"
    $EXECUTABLE $params $imgpath $tmpout 1> out.log
    exitcode=$?

    if [ $exitcode -ne 0 ]; then
        echo "Error while executing shading removal, exit code: $exitcode"
        return -1;
    fi

    local output=`cat out.log`
    rm out.log

    local paramvalues=`echo $output | cut -d, -f6`
    paramsdir="${PARAMNAME}_${paramvalues}"
    local outdir="${OUTPUT_DIR}/${paramsdir}"
    if [ ! -d "$outdir" ]; then
        mkdir $outdir;
    fi

    local outfile="${imgfile}.1out.jpg"
    local outpath="$outdir/$outfile"

    if [ -f "$outpath" ]; then
       echo "Output file '$outpath' already exists"
#      return 0
    fi

    mv $tmpout $outpath

#K_K04_MC16_GD08_SL08_DBUC04_PBUC61_VP25,diagonallights_beige_pag014_livememory_bgEDE7D5.jpg.light.1out.jpg,K,4,1856,2930,72945,60671,13444,K04_MC16_GD08_SL08_DBUC04_PBUC61_VP25,ede7d5,0.234001,1.3416,1.4196,1.8096,2.2464,2.96401
    echo "${paramsdir},${outfile},${PARAMNAME},${PARAMVALUE},${output}" >> "$REPORT"

    col=0
    echo "insert into shading_results set " >> "$SQL"
    insert_field "file" "varchar(240)" "$outfile"
    insert_field "imgW" "int" $(echo $output|cut -d, -f1)
    insert_field "imgH" "int" $(echo $output|cut -d, -f2)
    insert_field "blocks" "int" $(echo $output|cut -d, -f3)
    insert_field "BUC" "int" $(echo $output|cut -d, -f4)
    insert_field "delBUC" "int" $(echo $output|cut -d, -f5)
    insert_field "BG" "char(10)" $(echo $output|cut -d, -f7)

#        ngb,nn comp.,cluster selected,added to delaunay,grid estimated,done
    insert_field "time_1buc" "decimal" $(echo $output|cut -d, -f8)
    insert_field "time_2knn" "decimal" $(echo $output|cut -d, -f9)
    insert_field "time_3cluster" "decimal" $(echo $output|cut -d, -f10)
    insert_field "time_4del" "decimal" $(echo $output|cut -d, -f11)
    insert_field "time_5grid" "decimal" $(echo $output|cut -d, -f12)
    insert_field "time_6final" "decimal" $(echo $output|cut -d, -f13)

    insert_param "K" "int" $(echo $paramvalues|cut -d_ -f1)
    insert_param "MC" "int" $(echo $paramvalues|cut -d_ -f2)
    insert_param "GD" "int" $(echo $paramvalues|cut -d_ -f3)
    insert_param "SL" "int" $(echo $paramvalues|cut -d_ -f4)
    insert_param "DBUC" "int" $(echo $paramvalues|cut -d_ -f5)
    echo ";" >> "$SQL"

    #echo $output
    #echo $paramvalues
    #exit 1


}

executeparam() {

    local imgpath=$1
    local paramprefix=$2
    local begin=$3
    local end=$4
    local step=$5

    for ((i=$begin;i<=$end;i=$i+$step)); do
        getparams "${DEFAULT_PARAMS}" "${paramprefix}${i}"
        execute "$imgpath" "${params}"
    done

}

executeallparams() {

   #executeparam $1 "--K=" 4 8 1
#   executeparam $1 "--GD=" 8 32 4
#   executeparam $1 "--SL=" 8 32 4
#   bgcolor="INFER"
   bgcolor="FFFFFF"
#   executeparam $1 "--GD=" 8 24 4
   executeparam $1 "--GD=" 8 8 4
#   bgcolor="FFFFFF"
#   executeparam $1 "--GD=" 8 24 4

}

for imgpath in $(ls $DATABASE_DIR/*.JPG); do
#for imgpath in $(ls $DATABASE_DIR/*.light.*); do

    echo "########################################################"
    executeallparams $imgpath

done