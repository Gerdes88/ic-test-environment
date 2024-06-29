import pytest
import serial
import time

@pytest.fixture
def serial_connection():
    ser = serial.Serial('COM5', 9600, timeout=1)  # Ændret til COM5
    time.sleep(2)  # Allow time for Arduino to reset
    yield ser
    ser.close()

def test_temperature_format(serial_connection):
    serial_connection.reset_input_buffer()
    data = serial_connection.readline().decode('ascii').strip()
    assert data.startswith("Temperature: ")
    assert " C or " in data
    assert " F" in data

def test_temperature_range(serial_connection):
    serial_connection.reset_input_buffer()
    data = serial_connection.readline().decode('ascii').strip()
    temp_c = float(data.split()[1])
    temp_f = float(data.split()[4])
    assert -40 <= temp_c <= 125  # Typical range for DS18B20
    assert -40 <= temp_f <= 257  # Corresponding Fahrenheit range

def test_temperature_conversion(serial_connection):
    serial_connection.reset_input_buffer()
    data = serial_connection.readline().decode('ascii').strip()
    temp_c = float(data.split()[1])
    temp_f = float(data.split()[4])
    assert abs((temp_c * 9/5 + 32) - temp_f) < 0.1  # Allow for small rounding differences