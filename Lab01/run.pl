#!/usr/bin/perl
use strict;
use warnings;

# Usage: ./run.pl ./test/testXX
# Spawns multiple `./process <id> <file>` according to the first line of the test file

# Clean log
`make log_reset`;

# First input line is the number of `./process` to spawn
my $file = $ARGV[0] or die "Usage: $0 <testfile>\n";
my @input_lines = <>;
my $num_processes = shift @input_lines;
chomp($num_processes);

# Spawn the processes and wait for them
my @pids;
for (my $i = 0; $i < $num_processes; $i++) {
	 my $pid = fork();
	 if (!defined $pid) {
		  die "Fork failed: $!";
	 } elsif ($pid == 0) {
		 exec("./process", $i, $file) or die "Exec failed: $!";
		 exit;
	 }
}
1 while wait() >= 0;

# Open log.txt and check consistency
open(my $log_fh, '<', 'log.txt') or die "Could not open log.txt: $!";
my @log_lines = <$log_fh>;
close($log_fh);

my $last_time = 0;
my $number_in_crit = 0;
my @in_critical; # per pid
my @number_locks_taken; # per pid
for my $l (@log_lines) {
	print $l;
	if($l =~ /\[Process (\d+)\] \[Time (\d+)\] Lock taken/) {
		$number_in_crit++;
		$in_critical[$1] = 1;
		$number_locks_taken[$1]++;

		die "Timestamps are not non-decreasing!\n" if($2 < $last_time);
		$last_time = $2;

		check_wait_constraints($1);
	} elsif($l =~ /\[Process (\d+)\] \[Time (\d+)\] Lock released/) {
		$number_in_crit--;
		die "Inconsistent log: more releases than takes!\n" if($number_in_crit < 0);

		if(!$in_critical[$1]) {
			die "Process $1 released lock without being in critical section!\n";
		} else {
			$in_critical[$1] = 0;
		}

		die "Timestamps are not non-decreasing!\n" if($2 < $last_time);
		$last_time = $2;
	} else {
		die "Unrecognized log line: $l";
	}
}

sub check_wait_constraints {
	my $pid = shift;
	my $locks_so_far = 0;
	my @wait_constraints;

	# Parse the input till the nth lock taken by this process
	# and remember what it should have waited for
	for my $line (@input_lines) {
		if($line =~ /(\d+) Wait (\d+)/) {
			next if($1 != $pid);
			$wait_constraints[$2]++;
		} elsif($line =~ /(\d+) Lock/) {
			next if($1 != $pid);
			$locks_so_far++;
			last if($locks_so_far == $number_locks_taken[$pid]);
		}
	}

	# Did the pid take too many locks?
	if($locks_so_far != $number_locks_taken[$pid]) {
		die "Process $pid took too many locks! (Only $locks_so_far Lock instructions in test file, $number_locks_taken[$pid] taken.)\n";
	}

	# Check if all wait constraints are satisfied
	return if(scalar(@wait_constraints) == 0);
	for my $i (0 .. $#wait_constraints) {
		if(defined $wait_constraints[$i] && $wait_constraints[$i] > 0) {
			if(!defined $number_locks_taken[$i] || $number_locks_taken[$i] < $wait_constraints[$i]) {
				die "Process $pid did not wait enough for process $i! (Waited for ".($number_locks_taken[$i]//0)." locks instead of $wait_constraints[$i].)\n";
			}
		}
	}
}
