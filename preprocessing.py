import csv

with open('sensor_data_labeled.csv', mode='r') as file:
    reader = csv.DictReader(file)
    for row in reader:
        print(row)
