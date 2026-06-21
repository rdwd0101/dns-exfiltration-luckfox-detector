import os

dataset_path = os.path.abspath(os.path.join(os.path.curdir, "training.csv"))
fill_dataset_path = os.path.abspath(os.path.join(os.path.curdir, "top-10000-domains.txt"))
with open(dataset_path, 'a') as f_out:
	with open(fill_dataset_path, 'r') as f_in:
		i = 0
		for line in f_in:
			clean_line = line.strip() 
			f_out.write("0,{}.\n".format(clean_line))
			i += 1
			if i > 5000:
				break
