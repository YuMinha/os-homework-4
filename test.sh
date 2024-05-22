# set -x
diff_max=0.00001
fn=hw4
nop=13

function missed_arg() {
    $1 $2 $3 $4 $5 2> temp.txt
    if [ $? -eq 0 ] ; then
	echo "Fail: Program should use 4 args"
	exit 1
    fi
    output=$(cat temp.txt)    
    if [ -z "${output}" ] ; then
	echo "Fail: No error message for incorrent arguments"
	exit 1
    fi        
    if [[ "${output}" != *"Usage"* ]] && [[ "${output}" != *"${fn}"* ]] ; then
	echo "Fail: Incorrect error message(${output})"
	exit 1
    fi
}
    
function invalid_arg() {
    $1 $2 $3 $4 $5 2> temp.txt
    if [ $? -eq 0 ] ; then
	echo "Fail: 4th parameter should be positive integer"
	exit 1
    fi
    output=$(cat temp.txt)    
    if [ -z "${output}" ] ; then
	echo "Fail: No error message for incorrent 4th arguments"
	exit 1
    fi        
    if [[ "${output}" != *"number"* ]] && [[ "${output}" != *"process"* ]] ; then
	echo "Fail: Incorrect error message(${output})"
	exit 1
    fi
}

function missed_arg_test() {
    missed_arg "./${fn}"
    
    missed_arg "./${fn} A2.dat"

    missed_arg "./${fn} A2.dat B2.dat"

    missed_arg "./${fn} A2.dat B2.dat X2.dat"

    missed_arg "./${fn} A2.dat B2.dat X2.dat ${nop} 2"

    invalid_arg "./${fn} A2.dat B2.dat X2.dat 0"

    invalid_arg "./${fn} A2.dat B2.dat X2.dat -2"
}

function square_test() {
    if [ ! -f notsq57.bin ] ; then
	echo "Fail: file 'notsq57.bin' does not exist."
	exit 1
    fi
    ./${fn} notsq57.bin B2.dat X2.dat ${nop} 2> temp.txt
    if [ $? -eq 0 ] ; then
	echo "Fail: Program cannot detect not square matrix file"
	exit 1
    fi
    output=$(cat temp.txt)
    if [ -z "${output}" ] ; then
	echo "Fail: No error message for incorrent arguments"
	exit 1
    fi        
    if [[ "${output}" != *"Error"* ]] && [[ "${output}" != *"not"* ]] && [[ "${output}" != *"square"* ]] ; then
	echo "Fail: Incorrect error message(${output}) for not square matrix file."
	exit 1
    fi
}

function file_size_t() {
    $1 $2 $3 2> temp.txt
    if [ $? -eq 0 ] ; then
	echo "Fail: Program cannot detect different file sizes"
	exit 1
    fi
    output=$(cat temp.txt)
    if [ -z "${output}" ] ; then
	echo "Fail: No error message for different file sizes"
	exit 1
    fi        
    if [[ "${output}" != *"Error"* ]] && [[ "${output}" != *"not"* ]] && [[ "${output}" != *"square"* ]] ; then
	echo "Fail: Incorrect error message(${output}) for different file size."
	exit 1
    fi
}

function file_size_test() {
    file_size_t "./${fn} A2.dat B10.dat X10.dat 1" 

    file_size_t "./${fn} A10.dat B2.dat X2.dat 2"
}

function nxn_test(){
    output=$(./${fn} A$1.dat B$1.dat X$1.dat ${nop})
    if [ $? -eq 0 ] ; then
	echo "Step 1 passes: ($1*$1) Program exited zero"
    else
	echo "Step 1 fails: ($1*$1) Program did not exit zero"
	exit 1
    fi

    outputd=$(./hwdiff X$1.dat X${1}a.dat ${diff_max})
    if [ $? -eq 0 ] ; then
	echo "Step 2 passes: ($1*$1) Output is correct"
    else
	echo "Step 2 fails: ($1*$1) Output is not expected result"
	exit 1
    fi
}

function thread_n_test(){
    ./${fn} A1024.dat B1024.dat X1024.dat $1 &
    pidorg=$!
    sleep 0.01
    output=$(ps aux | grep "./${fn}" | wc -l)
    if [ ${output} -le $1 ] ; then
	echo "Step 1 passes: Single process runs"
    else
	echo "Step 1 fails: Multiple processes are used"
	exit 1
    fi
    
    output2=$(ps | grep ${fn})
    if [ -z "${output2}" ] ; then
	echo "Step 2 fails: process finishes too early."
	exit 1
    fi
    PID=$(echo ${output2} | head -n1 | awk '{print $1;}')
    NT=$(ps huH p ${PID} | wc -l)
    if [ ${NT} -gt $1 ] ; then
	echo "Step 2 passes: More than $1 threads(${NT}) run"
    else
	echo "Step 2 fails: Too small threads(${NT}) run"
	exit 1
    fi

    wait $pidorg
    outputd=$(./hwdiff X1024.dat X1024a.dat ${diff_max})
    if [ $? -eq 0 ] ; then
	echo "Step 3 passes: (1024*1024) Output is correct"
    else
	echo "Step 3 fails: (1024*1024) Output is not expected result"
	exit 1
    fi
}

function thread_n_test2(){
    nop2=5
    output=$(./${fn} A1024.dat B1024.dat X1024.dat ${nop2})
    if [ $? -eq 0 ] ; then
	echo "Step 1 passes: Program finished normally."
    else
	echo "Step 1 fails: Program finished abnormally, exit(1)."
	exit 1
    fi

    noth=$(echo ${output} | awk '{print $1;}')
    nothpeak=$(echo ${output} | awk '{print $2;}')
    if [[ ${noth} -eq ${nop2} ]] ; then
	echo "Step 2 passes: The number(${noth}) of active threads is ${nop2}"
    else
	echo "Step 2 fails: The number(${noth}) of active threads is not ${nop2}"
	exit 1
    fi
    if [[ ${nothpeak} -ge ${nop} ]] ; then
	echo "Step 3 passes: The number of peaks ${nothpeak} is greater then ${nop2}"
    else
	echo "Step 3 fails: The number(${nothpeak}) of active threads is not ${nop2}"
	exit 1
    fi
}

 echo "Running tests..."
echo

case $1 in
    "1") missed_arg_test;;
    "2") square_test;;
    "3") file_size_test;;
    "4") nxn_test "2";;
    "5") nxn_test "10";;
    "6") nxn_test "100";;
    "7") nxn_test "1024";;
    "8") thread_n_test "4";;
    "9") thread_n_test2;;
    *) echo "Invalid argument ($1) for the script"
       exit 1;;
esac

echo "Test $1 passed."
exit 0
