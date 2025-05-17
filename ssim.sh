
SSIM="/C/Program Files/MATLAB/R2010a/bin/matlab"
IRFANVIEW="/C/Program Files (x86)/IrfanView/i_view32.exe"

REPORT=ssim-`date +%s`.csv

DATABASE_DIR=./CameraGroundtruth/ProcessingGroundtruth/output
OUTPUT_DIR=./output

export MATLABPATH="$MATLABPATH";"$pwd"

ssim() {

    imgpath=$1
    local imgfile=${imgpath##*/}
    imgfile_gt=${imgfile/.light./.nolight.}
    imgfile_gt=${imgfile_gt%.1out.*}
    imgpath_gt="${DATABASE_DIR}/${imgfile_gt}"

    if [ -f "${imgpath_gt}.png" ]; then
      imgpath_gt="${imgpath_gt}.png"
    fi

    if [ -f "${imgpath_gt}.jpg" ]; then
      imgpath_gt="${imgpath_gt}.jpg"
    fi
    

    #cp ${imgpath_gt} gt.jpg
    #cp ${imgpath} img.jpg
    #"$IRFANVIEW" "${imgpath_gt}" /convert=gt.tif
    #"$IRFANVIEW" "${imgpath}" /convert=img.tif

    "$SSIM" -nodesktop -r "addpath('$pwd'); ssim_index '$pwd/gt.tif' $pwd/img.tif;" > ssim.log
    exitcode=$?
    exit 1

    if [ $exitcode -ne 0 ]; then
        echo "Error while executing ssim comparison, exit code: $exitcode"
        return -1;
    fi


    local ssimR=`sed '2,2!d' ssim.log`
    local ssimG=`sed '3,3!d' ssim.log`
    local ssimB=`sed '4,4!d' ssim.log`

    echo "$imgpath,$ssimR,$ssimG,$ssimB" >> $REPORT;

}


for imgdir in $OUTPUT_DIR/*; do

    if [ -d $imgdir ]; then
        echo "Processing directory '${imgdir}'"
        for imgfile in $imgdir/*.1out.*; do
            ssim $imgfile
        done
    fi

done