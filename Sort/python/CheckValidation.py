import argparse
import subprocess

def ImportArrayFromFile(file_path):
    fd = open(file_path, "r")
    file_content = fd.read().split("\n")
    
    arr = []
    for item in file_content[1].split(" ")[:-1]:
        arr.append(int(item))

    return arr

def RunConsistentProgram(file_path):
    subprocess.run(["../a.out", f'{file_path}'])

def RunParallelProgram(file_path, n_proc):
    subprocess.run(["mpiexec", "-n", f'{n_proc}', "../main", f'{file_path}', f'{n_proc}'])

def ArrCmp(arr1, arr2):
    if (len(arr1) != len(arr2)):
        print("[-] Arrays have not equal length")
        print(f'array arr1 has length {len(arr1)} and array arr2 has length {len(arr2)}')
        return False
    N = len(arr1)
    for i in range(N):
        if (arr1[i] != arr2[i]):
            print("[-] Arrays have not equal elements on one index")
            print(f'arr1[{i}] = {arr1[i]} and arr2[{i}] = {arr2[i]}')
            return False
    return True

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file_path", help="*description*")
    args = parser.parse_args()

    file_path = ""
    if args.file_path != None:
        file_path = args.file_path
    else:
        print("[-] Error, no file with data")
        raise RuntimeError
    
    RunConsistentProgram(file_path)
    RunParallelProgram(file_path, 2)

    consistent_result = ImportArrayFromFile("./consistent.txt")
    parallel_result = ImportArrayFromFile("./parallel.txt")

    valid_array = ImportArrayFromFile(file_path)
    valid_array.sort()
    
    if (ArrCmp(consistent_result, valid_array) == True):
        print("Consistent result is valid")
    if (ArrCmp(parallel_result, valid_array) == True):
        print("Parallel result is valid")


if __name__ == '__main__':
    main()
