import numpy as np
import pandas as pd

# Define the input CSV file
input_csv_file = 'polygon-distance-benchmark.csv'

# Load and process the data
df = pd.read_csv(input_csv_file, delimiter=';', quotechar='"')

# Strip extra spaces from column names
df.columns = df.columns.str.strip()

# Convert 'NxM' column to numeric types
df['NxM'] = pd.to_numeric(df['NxM'], errors='coerce')

# Pivot the table to aggregate comparisons
agg_df = df.pivot_table(index='Scenario', columns='Type', values='AVG of 10 in [s]', aggfunc='sum')
agg_df = agg_df.fillna(0).rename(columns={
    'Brute-Force': 'Brute-Force[s]',
    'X-Sweep': 'X-Sweep[s]',
    'Y-Sweep': 'Y-Sweep[s]',
    'Fishnet': 'Fishnet[s]'
})
agg_df = agg_df.reset_index()

# Create a DataFrame for scenarios and NxM values
df_scenarios = df[['Scenario', 'NxM']].drop_duplicates().reset_index(drop=True)
table_df = pd.merge(df_scenarios, agg_df, on='Scenario', how='left')

# Reorder columns to make 'Fishnet[s]' the last column
column_order = ['Scenario', 'NxM', 'Brute-Force[s]', 'X-Sweep[s]', 'Y-Sweep[s]', 'Fishnet[s]']
table_df = table_df[column_order]

# Generate LaTeX code for a tabular environment
latex_code = '\\begin{table}[h]\n'
latex_code += '\\centering\n'
latex_code += '\\begin{tabular}{|l|r|r|r|r|r|}\n'
latex_code += '\\hline\n'
latex_code += 'Scenario & $n \\times m$ & Brute-Force[s] & X-Sweep[s] & Y-Sweep[s] & Fishnet[s] \\\\ \n'
latex_code += '\\hline\n'
precision = 5
epsilon = float(f"1e-{precision-1}")
# Add rows to the LaTeX code
for _, row in table_df.iterrows():
    # Extract measurement values and their names
    measurements = np.array([
        row['Brute-Force[s]'],
        row['X-Sweep[s]'],
        row['Y-Sweep[s]'],
        row['Fishnet[s]']
    ])

    # Determine the minimum value
    min_value = np.min(measurements)

    # Determine which values are equal to the minimum value
    min_indices = np.where(np.abs(measurements - min_value) < epsilon)[0]

    # Format measurement values with 5 decimal places and bold if they are minimum
    brute_force = f"\\textbf{{{row['Brute-Force[s]']*1000:,.2f}}}" if 0 in min_indices else f"{row['Brute-Force[s]']*1000:,.2f}"
    x_sweep = f"\\textbf{{{row['X-Sweep[s]']*1000:,.2f}}}" if 1 in min_indices else f"{row['X-Sweep[s]']*1000:.2f}"
    y_sweep = f"\\textbf{{{row['Y-Sweep[s]']*1000:,.2f}}}" if 2 in min_indices else f"{row['Y-Sweep[s]']*1000:.2f}"
    fishnet = f"\\textbf{{{row['Fishnet[s]']*1000:,.2f}}}" if 3 in min_indices else f"{row['Fishnet[s]']*1000:.2f}"

    # Format NxM with comma as thousand separator and 0 decimal places
    nxm = f"{row['NxM']:,}"  # Adds commas as thousand separators

    # Append the formatted row to the LaTeX code
    latex_code += f"{row['Scenario']} & {nxm} & {brute_force} & {x_sweep} & {y_sweep} & {fishnet} \\\\ \n"
    latex_code += '\\hline\n'

latex_code += '\\end{tabular}\n'
latex_code += '\\caption{Summary of Comparisons by Scenario}\n'
latex_code += '\\label{tab:scenario_comparisons}\n'
latex_code += '\\end{table}\n'

# Save LaTeX code to a .tex file
with open('benchmark-table.tex', 'w') as f:
    f.write(latex_code)
print(latex_code)
print("\n")
print("LaTeX code saved to 'benchmark-table.tex'")

