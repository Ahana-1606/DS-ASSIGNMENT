# 🏦 Bank Queue Simulation System (BankQSim)

## Description

This project simulates and compares two different bank queue management systems:

- **Single Queue System** - All customers join one queue and are served by the first available teller
- **Multiple Queue System** - Each teller has their own queue, and customers choose the shortest queue

## Features

- Discrete event simulation
- Random customer arrival times
- Variable service times
- Teller idle time simulation
- Comprehensive statistics collection
- Comparative analysis between queue systems

## Requirements

- C compiler (GCC recommended)
- Windows/Linux operating system
- Standard C libraries

## Installation

```bash
git clone https://github.com/Sagnik-rc/BankQSim.git
cd bankqsim
gcc -o bankqsim main.c -lm
```

## Usage

```bash
./bankqsim <numCustomers> <numTellers> <simulationTime> <averageServiceTime>
```

### Parameters

- `numCustomers`: Total number of customers to simulate
- `numTellers`: Number of tellers available
- `simulationTime`: Total simulation time in minutes
- `averageServiceTime`: Expected average service time per customer in minutes

### Example

```bash
./bankqsim 100 4 60 2.3
```

This runs a simulation with:
- 100 customers
- 4 tellers
- 60 minutes simulation time
- 2.3 minutes average service time

## Output

The program provides detailed statistics for both queue systems including:

- Average wait times
- Maximum wait times
- Standard deviation of wait times
- Teller utilization rates
- Total service times
- Total idle times
- Comparative analysis between the two systems

## Technical Details

### Key Components

- Event Queue Management
- Customer Queue Management
- Random Time Generation
- Statistics Collection
- Memory Management
- Error Handling

### Data Structures

- Event Queue (Priority Queue)
- Customer Queue (FIFO Queue)
- Teller Status Tracking
- Statistics Collection

## Memory Management

The program includes comprehensive memory management with:

- Dynamic allocation for queues and statistics
- Proper cleanup of all allocated resources
- Memory leak prevention
- Error checking for allocations

## Limitations

- Fixed number of tellers throughout simulation
- No support for teller breaks or shifts
- Simplified random distribution for service times

## Author

Sagnik Roy Chowdhury

## Contributing

Guidelines for contributing to this project:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Create a Pull Request

## Support

For issues and questions, please open an issue on the GitHub repository.

---
