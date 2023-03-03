# Use for merge ORTModule convergence debugging summary files

import re
import argparse
import pprint
import os

parser = argparse.ArgumentParser()
parser.add_argument('--path', type=str)

# we should use the order.txt generated by pytorch run, that means, we follow the pytorch topo order to compare activation results.
# Be noted that, some tensor handled in pytorch might be missing in ORT graph (if the activation is not used by others, which is pruned during export)
parser.add_argument('--order', type=str)

args = parser.parse_args()
d_path=args.path
o_path=args.order

order_file = open(o_path)
tensor_name_in_order = order_file.readlines()

for sub_dir_name in os.listdir(d_path):
    sub_dir_full_path = os.path.join(d_path, sub_dir_name)
    if os.path.isdir(sub_dir_full_path):
        merge_filename_for_sub_dir = os.path.join(d_path, f"merge_{sub_dir_name}_.txt")
        # Open merge_filename_for_sub_dir in write mode
        with open(merge_filename_for_sub_dir, 'w') as outfile:
            # Iterate through list
            for filename in tensor_name_in_order:
                filename = filename.rstrip('\n')
                full_filename = os.path.join(sub_dir_full_path, filename)

                if not os.path.exists(full_filename):
                    print(f"tensor {full_filename} not exist")
                    continue

                # Open each file in read mode
                with open(full_filename) as infile:
                    # read the data from file1 and
                    # file2 and write it in file3
                    outfile.write(infile.read())

                # Add '\n' to enter data of file2
                # from next line
                outfile.write("\n")
