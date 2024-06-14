"""Main entry point for running the package like `python -m cts1_ground_support`."""
import streamlit as st
from cts1_ground_support.gui.main_gui import render_main_screen

if __name__ == "__main__":
    st.set_page_config(layout="wide")
    render_main_screen()
