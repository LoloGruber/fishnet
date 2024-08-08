import pandas as pd
import matplotlib.pyplot as plt

# Load the data from the CSV file with appropriate delimiter and quotechar
df = pd.read_csv('polygon-distance-benchmark.csv', delimiter=';', quotechar='"')

# Strip extra spaces from column names
df.columns = df.columns.str.strip()

# Print the first few rows to verify the data
print(df.head())

# Check for expected column names
if 'NxM' not in df.columns:
    print("Error: 'NxM' column is missing. Available columns are:", df.columns)
    exit(1)

# Exclude largest scenario in plot

# Convert the 'NxM' and 'AVG of 10 in [s]' columns to numeric types
df['NxM'] = pd.to_numeric(df['NxM'], errors='coerce')
df['AVG of 10 in [s]'] = pd.to_numeric(df['AVG of 10 in [s]'], errors='coerce')

df['NxM_formatted'] = df['NxM'].apply(lambda x: '{:,}'.format(int(x)) if pd.notnull(x) else 'NaN')

# Concatenate 'Scenario' and formatted 'NxM' to form new Scenario labels
df['Scenario'] = df['Scenario'] + ' (n x m=' + df['NxM_formatted'] + ')'

# Create a DataFrame to use for sorting
df_sort_key = df[['Scenario', 'NxM']].drop_duplicates().set_index('Scenario')

# Create a pivot table with 'Scenario' as rows and 'Type' as columns
df_pivot = df.pivot(index='Scenario', columns='Type', values='AVG of 10 in [s]')

# Add the sort key to the pivot table
df_pivot = df_pivot.join(df_sort_key)

# Sort the pivot table based on the sort-key value
df_pivot_sorted = df_pivot.sort_values(by='NxM', ascending=True)

# Drop the 'NxM' column used for sorting
df_pivot_sorted = df_pivot_sorted.drop(columns='NxM')

# Plotting
ax = df_pivot_sorted.plot(kind='bar', figsize=(14, 8),zorder=3)

# Add labels and title
plt.ylabel('Average Computation Time log(s)', fontsize=16)
plt.xlabel('Settlement Scenario (n x m)', fontsize=16)
plt.xticks(rotation=45, ha="right", fontsize=14)
plt.legend(title='Procedure', fontsize=14, title_fontsize=14)
plt.tight_layout()
# #plt.title('Average Computation Time by Scenario and Procedure (Excluding Diagonal_XL_XL)')
plt.minorticks_on()
plt.grid(True,which="both", linestyle='--', linewidth=0.5, color='gray', zorder=0)
ax.yaxis.set_minor_locator(plt.LogLocator(base=10.0, subs='auto', numticks=10))
plt.yscale("log")

# Save and show the plot
plt.savefig("polygon-distance.pdf")