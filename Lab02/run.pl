#!/usr/bin/perl
use strict;
use warnings;

# Usage: ./run.pl ./test/testXX
# Spawns multiple `./process <id> <file>` according to the first line of the test file

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
		 # Redirect stdout to log$i.txt, comment to debug
		 close STDOUT;
		 open (STDOUT, '>', "log$i.txt") or die "Can't redirect stdout: $!";

		 # Execute the process for a maximum of 2 seconds
		 exec("timeout", 2, "./process", $i, $file) or die "Exec failed: $!";
	 }
}
1 while wait() >= 0;

for my $l (@input_lines) {
	print "Broadcast $1 to $2 processes\n"	if($l =~ /BCAST (\d+) (\d+)/);
}

# Open log$i.txt and check consistency
my %delivered;
for my $i (0 .. $num_processes - 1) {
	open(my $log_fh, '<', "log$i.txt") or die "Could not open log$i.txt: $!";
	my @log_lines = <$log_fh>;
	close($log_fh);

	for my $l (@log_lines) {
		if($l =~ /\[Process (\d+)\] DELIVERED (\d+)/) {
			$delivered{$2}++;
		}
	}
}

if(keys(%delivered) == 0) {
	die "No messages delivered\n";
}
if(keys(%delivered) != 1) {
	die "Multiple distinct messages delivered: " . join(", ", keys(%delivered)) . "\n";
}
if($delivered{(keys %delivered)[0]} != $num_processes) {
	die "Not all processes delivered the message " . (keys %delivered)[0] . "\n";
}
print "All processes delivered the message " . (keys %delivered)[0] . "\n";
