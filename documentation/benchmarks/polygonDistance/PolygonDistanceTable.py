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
    'BRUTE_FORCE': 'Brute-Force[s]',
    'SWEEP_LINE_X': 'X-Sweep[s]',
    'SWEEP_LINE_Y': 'Y-Sweep[s]',
    'DEFAULT': 'Fishnet[s]'
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
latex_code += 'Scenario & NxM & Brute-Force[s] & X-Sweep[s] & Y-Sweep[s] & Fishnet[s] \\\\ \n'
latex_code += '\\hline\n'

# Add rows to the LaTeX code
for _, row in table_df.iterrows():
    latex_code += f"{row['Scenario']} & {row['NxM']} & {row['Brute-Force[s]']} & {row['X-Sweep[s]']} & {row['Y-Sweep[s]']} & {row['Fishnet[s]']} \\\\ \n"
    latex_code += '\\hline\n'

latex_code += '\\end{tabular}\n'
latex_code += '\\caption{Summary of Comparisons by Scenario}\n'
latex_code += '\\label{tab:scenario_comparisons}\n'
latex_code += '\\end{table}\n'

# Save LaTeX code to a .tex file
with open('table_latex.tex', 'w') as f:
    f.write(latex_code)

print("LaTeX code saved to 'table_latex.tex'")
