#!/usr/bin/ruby

# Runs the first argument a number of times equal
# to the second argument. Takes the best solution
# and elapsed time from the stdout each run, and
# writes them to a file, along with the averages.

command = ARGV[0]
runs = ARGV[1].to_i

s = Array.new
t = Array.new

File.open("out.txt", "a") do |dest|
  dest.puts "Command: #{command}"
  dest.puts "Runs: #{runs}\n"
  dest.puts
  runs.times do |i|
    output = `#{command}`
    s << output.match(/found: ([0-9]+)/).captures[0].to_i
    t << output.match(/time: ([0-9]+)/).captures[0].to_i
    dest.puts "#{s[i]}   #{t[i]}"
  end
  s_bar = s.inject(:+).to_f / runs
  t_bar = t.inject(:+).to_f / runs
  dest.puts "-----------"
  dest.puts "#{s_bar.round(1)} #{t_bar.round(1)}"
  dest.puts
  dest.puts "====================================="
end
