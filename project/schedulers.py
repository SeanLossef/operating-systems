# Sean Lossef - losses
# Joe Om - omj
# Solomon Chau - chaus2

import sys
import math
import heapq

# Scheduler Parent Class
class Scheduler:
	def __init__(self, cpu, processes, name, remaining_time=False):
		self.name = name
		self.t = 0
		self.cpu = cpu
		self.cpu.link_scheduler(self)
		self.waiting_process = processes
		self.blocked_processes = []
		self.completed_processes = []
		self.ready_queue = []
		self.remaining_time = remaining_time
		self.preempting = False
		self.preemptions = 0

	# Called when a process has completed all bursts
	def completed_push(self, p):
		self.completed_processes.append(p)

	# Sets process to blocking on IO
	def blocked_push(self, p):
		self.blocked_processes.append(p)

	# Returns list of all processes which just completed their IO block
	def blocked_complete(self):
		complete = []
		for p in self.blocked_processes:
			if p.blocked():
				if p.done(self.t):
					p.next_burst().started_at = self.t
					p.ready()
					complete.append(p)
		for p in complete:
			self.blocked_processes.remove(p)
		return complete

	# Returns true if there are any processes in ready queue
	def has_ready(self):
		return len(self.ready_queue) > 0

	# Print message to console, only t 0->999, or if important
	def alert_parent(self, m, important=False):
		if self.t < 1000 or important:
			print(m)

	# Called on every clock cycle, returns True when complete
	def clock(self):
		# Start simulation
		if self.t == 0:
			for p in self.waiting_process:
				print(p)
			self.alert("Simulator started for "+self.name)

		# Iterate CPU
		running_complete = self.cpu.clock(self.t)
		if running_complete:
			p = self.cpu.remove_process()
			if p is not None:
				p.get_burst().terminated_at = self.t
				if p.complete():
					self.completed_push(p)
				else:
					self.blocked_push(p)

		if self.preempting:
			p = self.cpu.preempt_process()
			if p is not None:
				self.ready_queue_push(p)
				self.cpu.add_process(self.ready_queue_pop())
				self.preempting = False

		# Run scheduler specific actions in child class
		self.clock_child()

		# Start running process if CPU is not in use
		if not self.cpu.active():
			if self.has_ready():
				p = self.ready_queue_pop()
				self.cpu.add_process(p)

		# Check if simulator is complete
		if (not self.cpu.active()) and (not self.has_ready()) and (len(self.blocked_processes) == 0) and (len(self.waiting_process) == 0):
			self.alert("Simulator ended for "+self.name, True)
			self.analyze()
			return True

		self.t += 1

		return False

	# Perform analysis on simulation after its completion
	def analyze(self):
		burst_count = 0
		process_count = 0

		# Average CPU Burst Time
		burst = 0
		for p in self.completed_processes:
			for b in p.bursts:
				burst_count += 1
				burst += b.cpu
			process_count += 1
		burst /= burst_count

		# Average Process Turnaround Time
		turnaround = 0
		for p in self.completed_processes:
			for b in p.bursts:
				turnaround += b.terminated_at - b.started_at
		turnaround /= burst_count

		# Average Wait Time
		wait = turnaround - burst - (((burst_count + self.preemptions) * self.cpu.t_cs * 2) / burst_count)

		with open("simout.txt", "a") as f:
			print("Algorithm "+self.name, file=f)
			print("-- average CPU burst time: {:.3f} ms".format(burst), file=f)
			print("-- average wait time: {:.3f} ms".format(wait), file=f)
			print("-- average turnaround time: {:.3f} ms".format(turnaround), file=f)
			print("-- total number of context switches: {:d}".format(burst_count + self.preemptions), file=f)
			print("-- total number of preemptions: {:d}".format(self.preemptions), file=f)
			# for p in self.completed_processes:
			# 	for b in p.bursts:
			# 		print("CPU: "+str(b.cpu)+"ms,  TURNAROUND: "+str(b.terminated_at - b.started_at)+"ms,  Start: "+str(b.started_at)+"ms,  Terminated: "+str(b.terminated_at)+"ms", file=f)



# FCFS Scheduler
class FCFS(Scheduler):
	def __init__(self, cpu, processes):
		super().__init__(cpu, processes, "FCFS")

	def ready_queue_pop(self):
		return self.ready_queue.pop(0)

	def ready_queue_push(self, p):
		self.ready_queue.append(p)

	def alert(self, m, important=False):
		ready = [ p.pid for p in self.ready_queue ]
		self.alert_parent("time " + str(self.t) + "ms: " + m + " [Q " + ("<empty>" if not self.ready_queue else ' '.join(ready)) + "]", important)

	def clock_child(self):
		# Check status of blocked processes
		for p in self.blocked_complete():
			self.ready_queue_push(p)
			self.alert("Process "+p.pid+" completed I/O; added to ready queue")

		# Check for new arrivals
		for p in self.waiting_process:
			if p.arrival <= self.t:
				p.get_burst().started_at = self.t
				self.waiting_process.remove(p)
				self.ready_queue_push(p)
				self.alert("Process "+p.pid+" arrived; added to ready queue")




# SJF Scheduler
class SJF(Scheduler):
	def __init__(self, cpu, processes):
		super().__init__(cpu, processes, "SJF")

	def ready_queue_pop(self):
		return heapq.heappop(self.ready_queue)[2]

	# Sort priority queue by estimated job length
	def ready_queue_push(self, p):
		heapq.heappush(self.ready_queue, (p.get_tau(), p.pid, p))

	def alert(self, m, important=False):
		ready_sorted = []
		while self.ready_queue:
			ready_sorted.append(self.ready_queue_pop())
		for p in ready_sorted:
			self.ready_queue_push(p)
		ready = [ p.pid for p in ready_sorted ]
		self.alert_parent("time " + str(self.t) + "ms: " + m + " [Q " + ("<empty>" if not self.ready_queue else ' '.join(ready)) + "]", important)

	def clock_child(self):
		# Check status of blocked processes
		for p in self.blocked_complete():
			self.ready_queue_push(p)
			self.alert("Process "+p.pid+" (tau "+str(p.get_burst().tau)+"ms) completed I/O; added to ready queue")

		# Check for new arrivals
		for p in self.waiting_process:
			if p.arrival <= self.t:
				p.get_burst().started_at = self.t
				self.waiting_process.remove(p)
				self.ready_queue_push(p)
				self.alert("Process "+p.pid+" (tau "+str(p.get_burst().tau)+"ms) arrived; added to ready queue")




# SRT Scheduler
class SRT(Scheduler):
	def __init__(self, cpu, processes):
		super().__init__(cpu, processes, "SRT", True)

	def ready_queue_pop(self):
		return heapq.heappop(self.ready_queue)[2]

	# Sort priority queue by estimated remaining time
	def ready_queue_push(self, p):
		heapq.heappush(self.ready_queue, (p.remaining_tau(), p.pid, p))

	def alert(self, m, important=False):
		ready_sorted = []
		while self.ready_queue:
			ready_sorted.append(self.ready_queue_pop())
		for p in ready_sorted:
			self.ready_queue_push(p)
		ready = [ p.pid for p in ready_sorted ]
		self.alert_parent("time " + str(self.t) + "ms: " + m + " [Q " + ("<empty>" if not self.ready_queue else ' '.join(ready)) + "]", important)

	def clock_child(self):
		# Check status of blocked processes
		for p in self.blocked_complete():
			if self.cpu.active() and (p.remaining_tau() <= self.cpu.running_process.remaining_tau()) and not self.cpu.switching > 0:
				self.ready_queue_push(p)
				self.alert("Process "+p.pid+" (tau "+str(p.get_burst().tau)+"ms) completed I/O; preempting "+self.cpu.running_process.pid)
				self.preempting = True
				self.cpu.preempt_process()
			else:
				self.ready_queue_push(p)
				self.alert("Process "+p.pid+" (tau "+str(p.get_burst().tau)+"ms) completed I/O; added to ready queue")

		# Check if ready queue should preempt running process
		if self.has_ready():
			p = self.ready_queue_pop()
			if self.cpu.active() and (p.remaining_tau() <= self.cpu.running_process.remaining_tau()) and not self.cpu.switching > 0:
				self.ready_queue_push(p)
				self.alert("Process "+p.pid+" (tau "+str(p.get_burst().tau)+"ms) will preempt "+self.cpu.running_process.pid)
				self.preempting = True
				self.cpu.preempt_process()
			else:
				self.ready_queue_push(p)

		# Check for new arrivals
		for p in self.waiting_process:
			if p.arrival <= self.t:
				p.get_burst().started_at = self.t
				self.waiting_process.remove(p)
				self.ready_queue_push(p)
				self.alert("Process "+p.pid+" (tau "+str(p.get_burst().tau)+"ms) arrived; added to ready queue")




# RR Scheduler
class RR(Scheduler):
	def __init__(self, cpu, processes, t_slice, rr_add):
		super().__init__(cpu, processes, "RR", True)
		self.t_slice = t_slice
		self.rr_add = rr_add

	def ready_queue_pop(self):
		return self.ready_queue.pop(0)

	def ready_queue_push(self, p, rr_add=False):
		if rr_add and self.rr_add == "BEGINNING":
			self.ready_queue.insert(0, p)
		else:
			self.ready_queue.append(p)

	def alert(self, m, important=False):
		ready = [ p.pid for p in self.ready_queue ]
		self.alert_parent("time " + str(self.t) + "ms: " + m + " [Q " + ("<empty>" if not self.ready_queue else ' '.join(ready)) + "]", important)

	def clock_child(self):
		# Check time slice
		if self.cpu.active() and self.cpu.switching == 0 and self.cpu.progress - 1 == self.t_slice:
			if self.has_ready():
				self.alert("Time slice expired; process "+self.cpu.running_process.pid+" preempted with "+str(self.cpu.running_process.remaining_time()+1)+"ms to go")
				self.preempting = True
				self.cpu.preempt_process()
			else:
				self.alert("Time slice expired; no preemption because ready queue is empty")

		# Check status of blocked processes
		blocked = [(p.pid, p) for p in self.blocked_complete()]
		blocked.sort()
		for p in blocked:
			self.ready_queue_push(p[1], True)
			self.alert("Process "+p[1].pid+" completed I/O; added to ready queue")

		# Check for new arrivals
		for p in self.waiting_process:
			if p.arrival <= self.t:
				p.get_burst().started_at = self.t
				self.waiting_process.remove(p)
				self.ready_queue_push(p, True)
				self.alert("Process "+p.pid+" arrived; added to ready queue")