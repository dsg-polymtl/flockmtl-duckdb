# FlockMTL Notebook Setup

## Installation
First, install Jupytext:
```bash
pip install jupytext
```

## Kaggle API Configuration
1. Visit [Kaggle Account Settings](https://www.kaggle.com/settings)
2. Create a new API token to download the `kaggle.json` file
3. Place the file in the appropriate location:
   - Linux/Mac: `~/.kaggle/kaggle.json`
   - Windows: `C:\Users\<Windows-Username>\.kaggle\kaggle.json`

4. Set file permissions:
   ```bash
   chmod 600 ~/.kaggle/kaggle.json
   ```

## Jupytext Workflow

### Notebook Conversion
```bash
# Convert Python script to Jupyter Notebook
jupytext filename.py --to notebook
```
