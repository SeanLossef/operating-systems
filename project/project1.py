import sys
import math

# Random Object
class Rand48:
	def __init__(self, seed, lamb, u_bound):
		self.lamb = lamb
		self.n = (seed << 16) + 0x330e
		self.u_bound = u_bound

	def seed(self, seed):
		self.n = seed

	def next(self):
		self.n = (25214903917 * self.n + 11) & (2**48 - 1)
		return self.n

	def drand(self):
		return self.next() / 2**48

	def exp_rand(self):
		r = self.u_bound + 1
		while r > self.u_bound:
			r = -math.log( self.drand() ) / self.lamb
		return r



# CPU Object -- Represents an operating system, contains list of processes, controls which processes are run
class CPU:
	def __init__(self, processes, t_cs):
		self.t = 0
		self.t_cs = t_cs
		self.waiting_process = processes
		self.running_process = None
		self.blocked_process = []
		self.ready_queue = []
		self.switching = 0
		for p in processes:
			print(p)
		self.alert("Simulator started for FCFS")

	# Called upon process arrival
	def arrives(self, p):
		self.ready_queue.append(p)
		self.alert("Process "+p.pid+" arrived; added to ready queue")

	# Called to start a CPU burst
	def start_burst(self, p):
		self.t += self.t_cs
		self.running_process = p
		self.alert("Process "+p.pid+" started using the CPU for "+str(p.get_burst().cpu)+"ms burst")

	# Simulate 1 ms of CPU time
	def clock(self):
		# Check for new arrivals
		for p in self.waiting_process:
			if p.arrival <= self.t:
				self.waiting_process.remove(p)
				self.arrives(p)

		# Do work on processes
		if self.running_process is not None:
			if self.running_process.clock():
				io_time = self.running_process.block()

				if self.running_process.complete():
					self.alert("Process "+self.running_process.pid+" terminated")
				else:
					bursts_left = self.running_process.bursts_left()
					self.alert("Process "+self.running_process.pid+" completed a CPU burst; "+str(bursts_left)+" burst"+("" if bursts_left == 1 else "s")+" to go")
					self.alert("Process "+self.running_process.pid+" switching out of CPU; will block on I/O until time "+str(self.t + io_time + self.t_cs)+"ms")
					self.blocked_process.append(self.running_process)
				self.running_process = None

		# Iterate blocked processes
		for p in self.blocked_process:
			if p.clock():
				p.ready()
				self.blocked_process.remove(p)
				self.ready_queue.append(p)
				self.t += self.t_cs
				self.alert("Process "+p.pid+" completed I/O; added to ready queue")

		if self.switching == 0:
			# Start running process if CPU is not in use
			if len(self.ready_queue) > 0 and self.running_process == None:
				self.start_burst(self.ready_queue.pop(0))

		if self.switching > 0:
			self.switching -= 1
		self.t += 1

	# Returns true if all processes have completed
	def complete(self):
		return (self.running_process is None) and (len(self.ready_queue) == 0) and (len(self.blocked_process) == 0) and (len(self.waiting_process) == 0)

	def alert(self, m):
		ready = [ p.pid for p in self.ready_queue ]
		print("time " + str(self.t) + "ms: " + m + " [Q " + ("<empty>" if not self.ready_queue else ' '.join(ready)) + "]")


# Burst Object -- Times for a single CPU burst of a process
class Burst:
	def __init__(self, cpu, io):
		self.cpu = cpu
		self.io = io


# Process Object -- Information about a process burst times and current state
class Process:
	def __init__(self, pid, arrival):
		self.pid = pid
		self.arrival = arrival
		self.bursts = []
		self.current_burst = 0
		self.state = "cpu"
		self.progress = 0

	def add_burst(self, burst):
		self.bursts.append(burst)

	# Sets process state to blocked on IO, returns blocking time
	def block(self):
		self.state = "io"
		self.progress = 0
		return self.bursts[self.current_burst].io

	# Sets process state to cpu, iterates burst count
	def ready(self):
		self.state = "cpu"
		self.progress = 0
		self.current_burst += 1

	# Returns current burst
	def get_burst(self):
		return self.bursts[self.current_burst]

	# Returns number of bursts not completed
	def bursts_left(self):
		return len(self.bursts) - self.current_burst - 1

	# Returns true if the process has completed all bursts
	def complete(self):
		return len(self.bursts) == self.current_burst + 1 and self.state == "io"

	# Returns true if current burst is complete
	def clock(self):
		self.progress += 1

		if self.state == "cpu":
			return self.bursts[self.current_burst].cpu <= self.progress
		if self.state == "io":
			return self.bursts[self.current_burst].io < self.progress

	def __str__(self):
		return "Process " + self.pid + " [NEW] (arrival time " + str(self.arrival) + " ms) " + str(len(self.bursts)) + " CPU bursts"



def main():
	if len(sys.argv) != 8 and len(sys.argv) != 9:
		print("Usage: python3 project1.py seed lambda u_bound n t_cs alpha t_slice [rr_add]")
		return

	# Get command line arguments
	seed    = int(sys.argv[1])
	lamb    = float(sys.argv[2])
	u_bound = int(sys.argv[3])
	n       = int(sys.argv[4])
	t_cs    = int(int(sys.argv[5]) / 2)
	alpha   = sys.argv[6]
	t_slice = sys.argv[7]
	rr_add  = sys.argv[8] if len(sys.argv) == 9 else "END"

	r = Rand48(seed, lamb, u_bound)

	processes = []

	# Generate Pseudo-Random Processes
	for i in range(n):
		pid = chr(i + 65)
		arrival = math.floor(r.exp_rand())
		num_bursts = math.ceil(r.drand() * 100)

		p = Process(pid, arrival)

		for j in range(num_bursts):
			cpu_t = math.ceil(r.exp_rand())
			io_t  = math.ceil(r.exp_rand()) if j != num_bursts-1 else 0
			b = Burst(cpu_t, io_t)
			p.add_burst(b)

		processes.append(p)

	# FCFS ALGORITHM
	cpu = CPU(processes, t_cs)
	while not cpu.complete():
		cpu.clock()
	cpu.alert("Simulator ended for FCFS")


if __name__== "__main__":
	main()