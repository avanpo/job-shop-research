#!/usr/bin/ruby

# Converts instances from jobshop1.txt to the format
# used by my code.
#
# USAGE:
# Run with the source instance file name as parameter
# (should be in the same folder). The output will be
# saved to the parent folder using the same filename.

machines = 0
j = Array.new

# read file in machine and job arrays
File.foreach("#{ARGV[0]}") do |src|
  o = Array.new
  src.split.map(&:to_i).each_slice(2) do |op|
    machines = op.first + 1 if op.first + 1 > machines 
    op << 0
    o << op
  end
  j << o
end

m = Array.new(machines, 1)

# output file
File.open("../#{ARGV[0]}", "a") do |dest|
  dest.puts "types #{m.size}"
  m.each do |val|
    dest.puts "#{val}"
  end
  dest.puts "jobs #{j.size}"
  j.each do |job|
    dest.puts "ops #{job.size}"
    job.each do |op|
      dest.puts "#{op[0]} #{op[1]} #{op[2]}"
    end
  end
end
