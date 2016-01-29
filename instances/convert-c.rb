#!/usr/bin/ruby

# Converts instance files from the previous research
# project into the format used by my code.
#
# USAGE:
# Run with the source instance file name as parameter
# (should be in the same folder). The output will be
# saved to the parent folder using the same filename.

m = Array.new
j = Array.new
o = Array.new
op = Array.new

# read file in machine and job arrays
File.foreach("#{ARGV[0]}") do |src|
  if match = src.match(/machineCount : ([0-9]+)/)
    m += match.captures
  elsif /job/ =~ src
    unless o.empty?
      j << o
      o = Array.new
    end
  elsif match = src.match(/operationType : type([0-9]+)/)
    op << match.captures.first.to_i - 1
  elsif match = src.match(/processingTime : ([0-9]+)/)
    op << match.captures.first.to_i
  elsif match = src.match(/idleTimeAfter : ([0-9]+)/)
    op << match.captures.first.to_i
    o << op
    op = Array.new
  end
end
j << o

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
