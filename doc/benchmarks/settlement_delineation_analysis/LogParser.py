import os
import json
import csv

def load_logs(directory):
    return [
        os.path.join(directory, file)
        for file in os.listdir(directory)
        if file.endswith('stdout.log')
    ]


def get_output_filename(log_directory):
    """
    Retrieves the output filename from the MERGE job log file.
    
    :param log_directory: Directory path containing log files for MERGE job type.
    :return: Output filename as a string.
    """
    log_files = load_logs(log_directory)
    
    for log_file in log_files:
        with open(log_file, 'r') as f:
            data = json.load(f)
            if data['type'] == 'MERGE' and 'output' in data:
                if not data['output'].endswith("_edges.shp"):
                    return os.path.splitext(os.path.basename(data['output']))[0]
    
    return None

def parse_log_files_to_csv(job_type_directory, output_file_stem, csv_output_directory):
    """
    Parses log files and writes specified fields to a CSV based on job type.
    
    :param job_type_directory: Directory path for a specific job type (e.g., 'filter.cwl').
    :param output_filename: Output filename (full path) from the MERGE job log file.
    :param csv_output_directory: Directory path where CSV files will be saved.
    """
    
    # Define field mappings for each job type
    field_mapping = {
        'filter': [ 'output','polygon count', 'duration[s]'],
        'neighbours': ['primary-input','Adjacencies','duration[s]'],
        'components': ['Connected Components','duration[s]'],  
        'contraction': ['output','#Nodes-before-contraction','#Nodes-after-contraction','duration[s]'], 
        'split': ['duration[s]'],  
        'merge': ['output','duration[s]'],
        'analysis':['output','duration[s]']  
    }
    
    job_type = os.path.splitext(os.path.basename(job_type_directory))[0]
    fields = field_mapping.get(job_type, [])
    
    csv_filename = os.path.join(csv_output_directory, f"{output_file_stem}_{job_type}.csv")
    with open(csv_filename, mode='w', newline='') as file:
        writer = csv.writer(file)
        
        # Write header
        if fields:
            writer.writerow(fields)
        
        # Process log files
        log_files = load_logs(job_type_directory)
        
        for log_file in log_files:
            with open(log_file, 'r') as f:
                data = json.load(f)
                
                row = []
                for field in fields:
                    keys = field.split('.')
                    value = data
                    for key in keys:
                        value = value.get(key, None)
                        if value is None:
                            break
                    if isinstance(value, list):
                        value = ', '.join(value)
                    row.append(value)
                
                writer.writerow(row)

def process_log_root_directory(log_root_directory, csv_output_directory):
    """
    Processes all job type directories within the log root directory.
    
    :param log_root_directory: Root directory path containing job type directories.
    :param csv_output_directory: Directory path where CSV files will be saved.
    """
    job_type_directories = [
        os.path.join(log_root_directory, directory)
        for directory in os.listdir(log_root_directory)
        if os.path.isdir(os.path.join(log_root_directory, directory))
    ]
    output_file_stem = "Evaluation"
    for job_type_directory in job_type_directories:
        name = get_output_filename(job_type_directory)
        if name:
            output_file_stem = name
    for job_type_directory in job_type_directories:
        parse_log_files_to_csv(job_type_directory, output_file_stem, csv_output_directory)

# Example usage
log_root_directory = "/home/lolo/Documents/results/Bolivia/logs"
csv_output_directory = "/home/lolo/Documents/fishnet/documentation/benchmarks/settlement_delineation_analysis"

process_log_root_directory(log_root_directory, csv_output_directory)
