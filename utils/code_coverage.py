import argparse
import csv
import json
import os
import sys

StatsForAllLines = {}
TotalUniqueLines = 0

widelands_basedir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

parser = argparse.ArgumentParser(description='Analyse GCOV code coverage results.')
parser.add_argument('-i', '--input', type=str, default='./json', help='Directory where to look for input files (default: ./json).')
parser.add_argument('-o', '--output', type=str, default='./report', help='Directory to write the CSV reports to (default: ./report).')
parser.add_argument('-d', '--details', type=bool, default=True, help='Whether to write per-line coverage report for each source file (default: true).')
parser.add_argument('-a', '--all', action='store_true', help='Also include statistics for system headers (default: false).')
parser.add_argument('-v', '--verbose', action='store_true', help='Print statistics to stdout (default: false).')
args = parser.parse_args()

def process(json_file_path):
	try:
		with open(json_file_path, 'r') as json_file:
			data = json.load(json_file)
			for source_file_object in data['files']:
				source_file_name = source_file_object['file']

				if not args.all and not source_file_name.startswith(widelands_basedir):
					continue

				if not source_file_name in StatsForAllLines:
					StatsForAllLines[source_file_name] = {}
				LineStats = StatsForAllLines[source_file_name]

				for line_object in source_file_object['lines']:
					line_number = line_object['line_number']
					if not line_number in LineStats:
						LineStats[line_number] = 0
						global TotalUniqueLines
						TotalUniqueLines += 1
					LineStats[line_number] += line_object['count']
	except Exception as e:
		print(f'ERROR: Could not process {json_file_path}:', e)

json_files = [f'{args.input}/{file}' for file in os.listdir(args.input) if file.endswith('.json')]
if len(json_files) == 0:
	print('ERROR: No input files found!')
	sys.exit(1);

if args.verbose:
	print(f'Found {len(json_files)} files.')
for file in json_files:
	if args.verbose:
		print(f'Processing: {file}')
	process(file)

csv_data = []

reports_dir = os.path.abspath(args.output)
os.makedirs(reports_dir, exist_ok=True)

hit_lines_total = 0
missed_lines_total = 0
for src_file in sorted(StatsForAllLines):
	hit_lines = 0
	missed_lines = 0
	for line in StatsForAllLines[src_file]:
		if StatsForAllLines[src_file][line] > 0:
			hit_lines += 1
			hit_lines_total += 1
		else:
			missed_lines += 1
			missed_lines_total += 1

	total = hit_lines + missed_lines

	if total > 0:
		coverage = hit_lines / total
		csv_data.append([src_file, total, hit_lines, missed_lines, str(coverage * 100) + '%'])

		if args.details:
			with open(src_file, mode='r') as src:
				src_lines = src.readlines()
				output_file = reports_dir + '/' + src_file.replace('/', '_').lstrip('_') + '.csv'
				with open(output_file, mode='w', newline='') as file:
					writer = csv.writer(file)
					writer.writerow(['Line', 'Count', 'Code'])
					for line in StatsForAllLines[src_file]:
						writer.writerow([line, StatsForAllLines[src_file][line], src_lines[line - 1].rstrip('\r\n')])

			if args.verbose:
				print(f'{src_file}: Found {total} lines total, hit {hit_lines}, miss {missed_lines}, coverage {coverage * 100}%. Detailed report was written to {output_file}.')
		else:
			if args.verbose:
				print(f'{src_file}: Found {total} lines total, hit {hit_lines}, miss {missed_lines}, coverage {coverage * 100}%.')

	else:
		if args.verbose:
			print(f'{src_file}: No lines found, skipping')

if TotalUniqueLines == 0:
	print('ERROR: No lines found!')
	sys.exit(2);

coverage_total = hit_lines_total / TotalUniqueLines

print(f'Total unit test code coverage: Found {TotalUniqueLines} lines total, hit {hit_lines_total}, missed {missed_lines_total}, coverage {coverage_total * 100}%.')

output_file = os.path.abspath(reports_dir + '/report.csv')
with open(output_file, mode='w', newline='') as file:
	writer = csv.writer(file)

	writer.writerow(['File', 'Total Lines', 'Hit Lines', 'Missed Lines', 'Coverage'])
	writer.writerow(['TOTAL', TotalUniqueLines, hit_lines_total, missed_lines_total, str(coverage_total * 100) + '%'])
	writer.writerows(csv_data)

	print(f'Code coverage report was written to {output_file}.')

