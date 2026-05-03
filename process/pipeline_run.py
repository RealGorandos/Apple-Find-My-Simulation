import os
import subprocess
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import argparse
import sys
import re
import shutil
import matplotlib.cm as cm

parser = argparse.ArgumentParser(description = 'CLI pipeline tool for FindMy simulator.')
parser.add_argument('--inet', type = str, required = False, metavar = '<INET_Path>', help = 'The path of the root directory of INET Framework')
parser.add_argument('--simu5g', type = str, required = False, metavar = '<Simu5G_Path>', help = 'The path of the root directory of Simu5G Framework')
args = parser.parse_args()


path_to_project = '../FindMy/simulations'

try:
    print(os.getenv('PATH').split(':'))
    scavetool_path = os.path.join(list(filter(lambda x: 'omnetpp-6.1/bin' in x, os.getenv('PATH').split(':')))[0], 'opp_scavetool')
except:
    print('*** You need to use "source setenv" in the root directory of OMNeT++ to load required environment variables before running the pipeline.')
    sys.exit()


INET_path = args.inet


Simu5G_path = args.simu5g

if INET_path == None:
    print('Use "--inet" argument to specify the directory of INET Framework.')
    sys.exit()

if Simu5G_path == None:
    print('Use "--simu5g" argument to specify the directory of Simu5G Framework.')
    sys.exit()

print('INET Path:\t' + INET_path)
print('Simu5G Path:\t' + Simu5G_path)

if not os.path.exists(path_to_project + '/results'):
    os.makedirs(path_to_project + '/results')

with open(path_to_project + '/omnetpp.ini') as f:
    omnetINI = f.read()

config_list = []
matches = re.finditer(r"\[Config (.*?)\]", omnetINI, re.MULTILINE)
for matchNum, match in enumerate(matches, start = 1):
    for groupNum in range(0, len(match.groups())):
        groupNum = groupNum + 1
        config_list.append(match.group(groupNum))

server_vec_extract_options = [
    '-F', 'CSV-R', '-f',
    '(module=~Network.server.app[0]) AND '
    '(name=~throughput:vector OR '
    'name=~successfulLookupAirTag:vector OR '
    'name=~unsuccessfulLookupAirTag:vector OR '
    'name=~successfulLookupIphone:vector OR '
    'name=~unsuccessfulLookupIphone:vector)'
]

clients_vec_extract_options = [
    '-F', 'CSV-R', '-f',
    '(module=~Network.apple_device_host[*].app[0]) AND '
    '(name=~throughput:vector OR '
    'name=~unkownAirtagFollow:vector OR '
    'name=~friendlyMessages:vector OR '
    'name=~endtoenddelay:vector)'
]

airtags_vec_extract_options = [
    '-F', 'CSV-R', '-f',
    '(module=~Network.airtag[*].app[0]) AND '
    '(name=~throughput:vector)'
]


def clean_screen():
    for i in range(100):
        print()

def clean_project():
    print('Cleaning FindMy project...')
    subprocess.run(['make', 'clean'], stderr = sys.stderr, stdout = sys.stdout)
    
    results_folder = path_to_project + '/results'
    if os.path.exists(results_folder):
        shutil.rmtree(results_folder)

def build_project():
    clean_project()

    print('Building FindMy project...')
    subprocess.run([
        'opp_makemake',
        '--deep',
        '-f',
        '-pINET',
        '-psimu5g',
        '-KINET4_4_PROJ=' + INET_path,
        '-KSIMU5G_1_2_1_PROJ=' + Simu5G_path,
        '-DINET_IMPORT',
        '-I' + os.path.join(INET_path, 'src'),
        '-I' + os.path.join(Simu5G_path, 'src'),
        '-L' + os.path.join(INET_path, 'src'),
        '-L' + os.path.join(Simu5G_path, 'src'),
        '-lINET',
        '-lsimu5g'],
        stderr = sys.stderr, stdout = sys.stdout)

    result = subprocess.run(['make'], stderr = sys.stderr, stdout = sys.stdout)

def run_configuration(config):
    if not os.path.exists(os.path.join('..', 'FindMy')):
        clean_screen()
        print('*** You need to build the simulator before running the simulations.')
        print()
        return
    exe_path = os.path.abspath(os.path.join('..', 'FindMy'))
    simulations_path = os.path.abspath(os.path.join('..', 'FindMy', 'simulations'))
    print(os.path.join('..', 'FindMy', 'simulations'))
    print('Running simulation of ' + config + '...')
    result = subprocess.run([
        'opp_run',
        '-r',
        '0',
        '-m',
        '-u',
        'Cmdenv',
        '-n',
        '{}:{}:{}:{}'.format('.', os.path.join('..', 'src'), os.path.join(INET_path, 'src'), os.path.join(Simu5G_path, 'src')),
        '-l',
        os.path.join('..', 'src', 'FindMy'),
        '-l',
        os.path.join(INET_path, 'src', 'INET'),
        '-l',
        os.path.join(Simu5G_path, 'src', 'simu5g'),
        '-c',
        config,
        os.path.join('.', 'omnetpp.ini')],
        stderr = sys.stderr, stdout = sys.stdout, cwd = os.path.join('..', 'FindMy', 'simulations'))
    os.makedirs(f'../exported-results/{config}', exist_ok=True)
    extract_csv_files(f'../FindMy/simulations/results/{config}/{config}.vec', f'../exported-results/{config}/server_{config}.csv', server_vec_extract_options)
    extract_csv_files(f'../FindMy/simulations/results/{config}/{config}.vec', f'../exported-results/{config}/clients_{config}.csv', clients_vec_extract_options)
    extract_csv_files(f'../FindMy/simulations/results/{config}/{config}.vec', f'../exported-results/{config}/airtags_{config}.csv', airtags_vec_extract_options)
    analyze_and_plot_server_stats(f'../exported-results/{config}/server_{config}.csv')
    analyze_and_plot_clients_stats(f'../exported-results/{config}/clients_{config}.csv')
    analyze_and_plot_airtags_stats(f'../exported-results/{config}/airtags_{config}.csv')

def run_all_configurations():
    if not os.path.exists(os.path.join('..', 'FindMy')):
        clean_screen()
        print('*** You need to build the simulator before running the simulations.')
        print()
        return
    print('Running all simulations...')
    for config in config_list:
        run_configuration(config)
    print('All simulations finished...')


def run_scavetool(input_file, output_file, command='x', extra_args=None):
    """
    Run opp_scavetool with specified parameters.

    :param input_file: Path to the input scalar/vector file (.sca, .vec).
    :param output_file: Path to save the output.
    :param command: The opp_scavetool command to run (default is 'x' for export).
    :param extra_args: List of additional arguments to pass to scavetool.
    """
    cmd = ['opp_scavetool', command, '-o', output_file, input_file]
    if extra_args:
        cmd.extend(extra_args)
    
    try:
        result = subprocess.run(cmd, check=True, capture_output=True, text=True)
        print("Success:", result.stdout)
    except subprocess.CalledProcessError as e:
        print("Error running opp_scavetool:", e.stderr)
    except FileNotFoundError:
        print("Error: opp_scavetool not found. Ensure it is in your PATH.")
    except Exception as ex:
        print(f"Unexpected error: {ex}")

def extract_csv_files(vec_file, output_csv_file, opp_scavetool_options):
    """Extracts CSV files using opp_scavetool."""
    if os.path.exists(vec_file):
        run_scavetool(vec_file, output_csv_file, extra_args=opp_scavetool_options)
    else:
        print(f"Error: {vec_file} does not exist")

def plot_server_statistics(dataframe, stat_names, title, xlabel, ylabel, output_filename, directory):
    """Helper function to plot and save statistics."""
    for name in stat_names:
        filtered_data = dataframe[dataframe['name'].str.contains(name)]
        if not filtered_data.empty:
            time_vector = np.array(list(map(float, filtered_data.iloc[0]['vectime'].split())))
            value_vector = np.array(list(map(float, filtered_data.iloc[0]['vecvalue'].split())))
            
            plt.title(title)
            plt.xlabel(xlabel)
            plt.ylabel(ylabel)
            plt.plot(time_vector, value_vector, label=name, linestyle='solid')
            plt.legend(loc='upper right', fontsize='small', ncol=2, framealpha=0.5)
            plt.grid()
            plt.savefig(f'{directory}/{output_filename}', bbox_inches='tight')
    plt.clf()

def analyze_and_plot_server_stats(csv_vec_file):
    """Analyzes and plots statistics from extracted CSV files."""
    try:
        if not os.path.exists(csv_vec_file):
            print(f"Error: {csv_vec_file} does not exist")
            return
        
        directory = os.path.dirname(csv_vec_file)
        df_vec = pd.read_csv(csv_vec_file)
        filtered_df_vec = df_vec[df_vec['type'] == 'vector']

        plot_server_statistics(
            dataframe=filtered_df_vec,
            stat_names=['throughput:vector'],
            title='Throughput of server',
            xlabel='Time (s)',
            ylabel='Throughput (bps)',
            output_filename='server-Throughput.png',
            directory=directory
        )

        plot_server_statistics(
            dataframe=filtered_df_vec,
            stat_names=['successfulLookupAirTag:vector', 'unsuccessfulLookupAirTag:vector'],
            title='Database Lookups (AirTag)',
            xlabel='Time (s)',
            ylabel='Number of responses',
            output_filename='server-Requests-Responses-airtag.png',
            directory=directory
        )

        plot_server_statistics(
            dataframe=filtered_df_vec,
            stat_names=['successfulLookupIphone:vector', 'unsuccessfulLookupIphone:vector'],
            title='Database Lookups (iPhone)',
            xlabel='Time (s)',
            ylabel='Number of responses',
            output_filename='server-Requests-Responses-iphone.png',
            directory=directory
        )
    
    except Exception as ex:
        print(f"Unexpected error: {ex}")



def plot_clients_statistics(dataframe, stat_names, title, xlabel, ylabel, output_filename, directory):
    """Helper function to plot and save statistics."""
    for name in stat_names:
        filtered_data = dataframe[dataframe['name'].str.contains(name)]
        if not filtered_data.empty:
            plt.figure(figsize=(18, 12)) 
            num_lines = len(filtered_data)
            count = 0
            colors = plt.get_cmap('turbo', num_lines)
            minY = 1000
            for index, df in filtered_data.iterrows():
                module_name = df['module'].split('.')[1]
                if pd.isna(df['vectime']) or pd.isna(df['vecvalue']):
                    time_vector = np.arange(0, 301) # Create a vector of zeros (you can modify the size)
                    value_vector = np.zeros(301)
                else:
                    time_vector = np.array(list(map(float, df['vectime'].split())))
                    value_vector = np.array(list(map(float, df['vecvalue'].split())))
                
                plt.title(title)
                plt.xlabel(xlabel)
                plt.ylabel(ylabel)
                if 'throughput' in name:
                    plt.plot(time_vector, value_vector, label=module_name, linestyle='solid', color=colors(count))
                elif 'endtoenddelay' in name:
                    plt.plot(time_vector, value_vector, label=module_name, linestyle='-', color=colors(count))
                else:
                    plt.plot(time_vector, value_vector, label=module_name, linestyle='-', marker='o', color=colors(count))
                
                plt.grid(True, linestyle='--', alpha=0.5)
                handles, labels = plt.gca().get_legend_handles_labels()
                plt.legend(handles, labels, loc='upper left', fontsize='small', ncol=2, framealpha=0.5)
                count = count + 1
            
            plt.savefig(f'{directory}/{output_filename}', bbox_inches='tight')
    plt.clf()

def analyze_and_plot_clients_stats(csv_vec_file):
    """Analyzes and plots statistics from extracted CSV files."""
    try:
        if not os.path.exists(csv_vec_file):
            print(f"Error: {csv_vec_file} does not exist")
            return
        
        directory = os.path.dirname(csv_vec_file)
        df_vec = pd.read_csv(csv_vec_file)
        filtered_df_vec = df_vec[df_vec['type'] == 'vector']
        print(len(filtered_df_vec))
        plot_clients_statistics(
            dataframe=filtered_df_vec,
            stat_names=['throughput:vector'],
            title='Throughput of clients',
            xlabel='Time (s)',
            ylabel='Throughput (bps)',
            output_filename='clients-Throughput.png',
            directory=directory
        )
        plot_clients_statistics(
            dataframe=filtered_df_vec,
            stat_names=['unkownAirtagFollow:vector'],
            title='Clients Followed Count by unkown AirTags',
            xlabel='Time (s)',
            ylabel='Amount of Times Followed',
            output_filename='clients-followed-count.png',
            directory=directory
        )
        plot_clients_statistics(
            dataframe=filtered_df_vec,
            stat_names=['friendlyMessages:vector'],
            title='Clients messages recieved by friendly iPhones',
            xlabel='Time (s)',
            ylabel='Amount of Messages Followed',
            output_filename='clients-messages-count.png',
            directory=directory
        )
        plot_clients_statistics(
            dataframe=filtered_df_vec,
            stat_names=['endtoenddelay:vector'],
            title='Clients end to end delay',
            xlabel='Time (s)',
            ylabel='End-To-End Delay (s)',
            output_filename='clients-e2e-delay.png',
            directory=directory
        )
    except Exception as ex:
        print(f"Unexpected error: {ex}")


def analyze_and_plot_airtags_stats(csv_vec_file):
    """Analyzes and plots statistics from extracted CSV files."""
    try:
        if not os.path.exists(csv_vec_file):
            print(f"Error: {csv_vec_file} does not exist")
            return
        
        directory = os.path.dirname(csv_vec_file)
        df_vec = pd.read_csv(csv_vec_file)
        filtered_df_vec = df_vec[df_vec['type'] == 'vector']
        print(len(filtered_df_vec))
        plot_clients_statistics(
            dataframe=filtered_df_vec,
            stat_names=['throughput:vector'],
            title='Throughput of airtags',
            xlabel='Time (s)',
            ylabel='Throughput (bps)',
            output_filename='airtags-Throughput.png',
            directory=directory
        )

    except Exception as ex:
        print(f"Unexpected error: {ex}")


def plot_available_config():
    for config in config_list:
        os.makedirs(f'../exported-results/{config}', exist_ok=True)
        extract_csv_files(f'../FindMy/simulations/results/{config}/{config}.vec', f'../exported-results/{config}/server_{config}.csv', server_vec_extract_options)
        extract_csv_files(f'../FindMy/simulations/results/{config}/{config}.vec', f'../exported-results/{config}/clients_{config}.csv', clients_vec_extract_options)
        extract_csv_files(f'../FindMy/simulations/results/{config}/{config}.vec', f'../exported-results/{config}/airtags_{config}.csv', airtags_vec_extract_options)
        analyze_and_plot_server_stats(f'../exported-results/{config}/server_{config}.csv')
        analyze_and_plot_clients_stats(f'../exported-results/{config}/clients_{config}.csv')
        analyze_and_plot_airtags_stats(f'../exported-results/{config}/airtags_{config}.csv')


selection = ""
menuOpen = True
while (menuOpen):
    print('==================================================================')
    print('You can run different configurations with this tool.')
    print('The configurations are getting from omnet.ini file directly.')
    print('To add new configurations, update the omnet.ini file accordingly.')
    print('You can use the following interactive menu for simulations.')
    print('==================================================================')
    print('Select the configuration that you would like to put into pipeline:')
    print('==================================================================')
    for i in range(len(config_list)):
        print(' ' + str(i + 1) + '.\t' + config_list[i])
    print('==================================================================')
    print(' A.\tRun everything!')
    print(' B.\tBuild the project.')
    print(' C.\tClean working directory.')
    print(' P.\tPlot all available configurations.')
    print(' Q.\tQuit.')
    print('==================================================================')

    selection = input("Your selection:").upper()
    
    if selection.isnumeric() and int(selection) >= 1 and int(selection) <= len(config_list):
        run_configuration(config_list[int(selection) - 1])
    elif selection == 'A':
        run_all_configurations()
    elif selection == 'B':
        build_project()
    elif selection == 'C':
        clean_project()
    elif selection == 'P':
        plot_available_config()
    elif selection == 'Q':
        menuOpen = False
    else:
        clean_screen()
        print('Unknown input!')
        print()
