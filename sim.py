import os
import subprocess
import time
import random

node_list = [4, 8, 10, 20, 30]

def phase_one(num_nodes, exp_num):
	commands = []
	nodes = []
	commands.append("detect\n")
	while len(nodes) is not node_list[exp_num]:
		var = random.randint(1, 255)
		if var not in nodes:
			random.seed(time.time())
			time.sleep(0.5)
			commands.append("join " + str(var) + "\n")
			nodes.append(var)
	return commands


def phase_two(num_nodes, exp_num):
	commands = []
	nodes = []
	commands.append("detect\n")
	for i in range(0, num_nodes[exp_num]):
		var = random.randint(1, 255)
		random.seed(time.time())
		time.sleep(0.5)
		commands.append("join " + str(var) + "\n")
		nodes.append(var)

	for i in range(0, 128):
		node = random.choice(nodes)
		random.seed(time.time())
		time.sleep(0.5)
		var = random.randint(0, 255)
		random.seed(time.time())
		time.sleep(0.5)
		commands.append("find " + str(node) + " " + str(var) + "\n")
	return commands


def main(cmd):
	p = subprocess.Popen(["./chord"], stdin=subprocess.PIPE, stdout = subprocess.PIPE, shell = False, close_fds = True)  #NOTE: no shell=True here

	time.sleep(1)
	for item in cmd:
		p.stdin.write(item)
		#print item
		time.sleep(1)

	p.stdin.write("print\nexit\n")
	output = p.stdout.read()
	print output

if __name__ == '__main__':
	main(phase_one(node_list, 0))
	#main(phase_two(node_list, 0))