import numpy as np

def CleanFile(file_path):
    fd = open(file_path, "w")
    fd.write("")
    fd.close()

def GenArray(low, high, size, file_path):
    CleanFile(file_path)

    gen_array = np.random.randint(low, high, size)
    gen_array_size = np.size(gen_array)
    
    fd = open(file_path, "w")
    fd.write(str(gen_array_size) + "\n")
    for item in gen_array:
        fd.write(str(item) + " ")
    fd.close()

def GenBadCase(low, high, size, file_path):
    CleanFile(file_path)

    gen_array = np.random.randint(low, high, size)
    gen_array_size = np.size(gen_array)

    gen_array = list(gen_array)
    gen_array.sort(reverse=True) 
    
    fd = open(file_path, "w")
    fd.write(str(gen_array_size) + "\n")
    for item in gen_array:
        fd.write(str(item) + " ")
    fd.close()

def main():
    low = -1000000
    high = 1000000
    file_path = "../tests/"

    # size_list = [
    #     50,    100,   200,   500,   1000,  2000,  3000,  5000,  8000,  
    #     10000, 12000, 14000, 16000, 18000, 20000, 22000, 24000, 26000,
    #     28000, 30000, 32000, 34000, 36000, 38000, 40000, 42000, 44000,
    #     46000, 48000, 50000, 52000, 56000, 58000, 60000, 62000, 64000
    # ]
    # size_list = [
    #     80000000,  100000000, 120000000, 140000000, 160000000,
    #     180000000, 200000000, 220000000, 240000000, 260000000
    # ]
    # size_list = [
    #     80000000,  100000000
    # ]
    size_list = [
        12
    ]

    for size in size_list:
        file_name = "test" + str(size) + ".dat"
        bad_case_file_name = "bad_case" + str(size) + ".dat"

        GenArray(low, high, size, file_path+file_name)
        GenBadCase(low, high, size, file_path+bad_case_file_name)


if __name__ == '__main__':
    main()