# Sean Lossef - losses
# Joe Om - omj
# Solomon Chau - chaus2

import sys
import math
import heapq

# Process Object -- Information about a process burst times and current state
class Process:
	def __init__(self, pid, arrival, alpha):
		self.pid = pid
		self.arrival = arrival
		self.bursts = []
		self.current_burst = 0
		self.state = "cpu"
		self.progress = 0
		self.alpha = alpha
		self.done_at = 0
		self.terminated_at = 0

	def add_burst(self, burst):
		self.bursts.append(burst)

	# Sets process state to blocked on IO, returns blocking time
	def block(self, t, t_cs):
		self.state = "io"
		self.progress = 0
		self.done_at = t + t_cs + self.bursts[self.current_burst].io
		return self.bursts[self.current_burst].io

	# Returns true if IO block is complete
	def done(self, t):
		return self.done_at <= t

	# Sets process state to cpu, iterates burst count
	def ready(self):
		self.state = "cpu"
		self.progress = 0
		self.current_burst += 1

	# Returns True if process is currently blocked on IO
	def blocked(self):
		return self.state == "io"

	# Returns current burst
	def get_burst(self):
		return self.bursts[self.current_burst]

	# Returns next burst
	def next_burst(self):
		return self.bursts[self.current_burst+1]

	# Returns number of bursts not completed
	def bursts_left(self):
		return len(self.bursts) - self.current_burst - 1

	# Returns tau of current burst
	def get_tau(self):
		return self.bursts[self.current_burst].tau

	# Returns true if the process has completed all bursts
	def complete(self):
		return len(self.bursts) == self.current_burst + 1 and self.bursts[self.current_burst].cpu < self.progress

	# Returns real time remaining on current burst
	def remaining_time(self):
		return self.get_burst().cpu - self.progress

	# Returns estimated time remaining on current burst
	def remaining_tau(self):
		return self.get_burst().tau - self.progress

	# Returns true if current burst is complete
	def clock(self):
		self.progress += 1

		if self.complete():
			return True
		if self.state == "cpu":
			return self.bursts[self.current_burst].cpu < self.progress
		if self.state == "io":
			return self.bursts[self.current_burst].io < self.progress

	def __str__(self, tau=False):
		ret = "Process " + self.pid + " [NEW] (arrival time " + str(self.arrival) + " ms) " + str(len(self.bursts)) + " CPU bursts"
		if self.alpha is not None:
			ret += " (tau "+str(self.bursts[0].tau)+"ms)"
		return ret


# Burst Object -- Times for a single CPU burst of a process
class Burst:
	def __init__(self, cpu, io, tau):
		self.cpu = cpu
		self.io = io
		self.tau = tau
		self.started_at = 0
		self.terminated_at = 0


# CPU Object
class CPU:
	def __init__(self, t_cs):
		self.running_process = None
		self.t = 0
		self.t_cs = t_cs
		self.switching = 0
		self.removing = False
		self.preempting = False
		self.progress = 0

	def link_scheduler(self, scheduler):
		self.scheduler = scheduler

	# Returns true if running process burst is complete
	def clock(self, t):
		self.t = t

		if self.active():
			if self.switching > 1:
				self.switching -= 1
			elif self.switching == 1:
				self.switching -= 1
				self.progress = 0
				if not self.removing and not self.preempting:
					if self.running_process.alpha is not None:
						if self.scheduler.remaining_time:
							self.scheduler.alert("Process "+self.running_process.pid+" (tau "+str(self.running_process.get_burst().tau)+"ms) started using the CPU with "
								+str(self.running_process.remaining_time())+"ms burst remaining")
						else:
							self.scheduler.alert("Process "+self.running_process.pid+" (tau "+str(self.running_process.get_burst().tau)+"ms) started using the CPU for "
								+str(self.running_process.get_burst().cpu)+"ms burst")
					else:
						if self.scheduler.remaining_time and self.running_process.progress > 0:
							self.scheduler.alert("Process "+self.running_process.pid+" started using the CPU with "
								+str(self.running_process.remaining_time())+"ms burst remaining")
						else:
							self.scheduler.alert("Process "+self.running_process.pid+" started using the CPU for "+str(self.running_process.get_burst().cpu)+"ms burst")
			
			if self.switching == 0:
				if not self.removing and not self.preempting:
					complete = self.running_process.clock()
					self.progress += 1
					if complete:
						bursts_left = self.running_process.bursts_left()
						if self.running_process.complete():
							self.scheduler.alert("Process "+self.running_process.pid+" terminated", True)
							self.running_process.terminated_at = self.t
						else:
							if self.running_process.alpha is not None:
								self.scheduler.alert("Process "+self.running_process.pid+" (tau "+str(self.running_process.get_burst().tau)+"ms) completed a CPU burst; "
									+str(bursts_left)+" burst"+("" if bursts_left == 1 else "s")+" to go")
								self.scheduler.alert("Recalculated tau = "+str(self.running_process.next_burst().tau)+"ms for process "+self.running_process.pid)
							else:
								self.scheduler.alert("Process "+self.running_process.pid+" completed a CPU burst; "+str(bursts_left)+" burst"
									+("" if bursts_left == 1 else "s")+" to go")
						return True
				elif self.preempting:
					return False
				else:
					return True

		return False

	def add_process(self, p):
		self.running_process = p
		self.switching = self.t_cs

	def remove_process(self):
		if self.removing and self.switching == 0:
			self.removing = False
			p = self.running_process
			self.running_process = None
			return p
		else:
			if not self.running_process.complete():
				io_time = self.running_process.block(self.t, self.t_cs)
				self.scheduler.alert("Process "+self.running_process.pid+" switching out of CPU; will block on I/O until time "+str(self.t + io_time + self.t_cs)+"ms")
			self.removing = True
			self.switching = self.t_cs
			return None

	def preempt_process(self):
		# if self.running_process is not None:
		# 	self.running_process.progress -= 1
		if self.preempting and self.switching == 0:
			self.preempting = False
			p = self.running_process
			self.running_process = None
			return p
		elif not self.preempting:
			self.scheduler.preemptions += 1
			self.running_process.progress -= 1
			self.preempting = True
			self.switching = self.t_cs
			return None

	def active(self):
		return self.running_process is not None