-- phpMyAdmin SQL Dump
-- version 4.9.0.1
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Generation Time: Jul 29, 2019 at 09:07 PM
-- Server version: 10.1.13-MariaDB
-- PHP Version: 7.3.1p1

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `iotsv`
--

-- --------------------------------------------------------

--
-- Table structure for table `devices`
--

CREATE TABLE `devices` (
  `idx` int(11) NOT NULL,
  `update_time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `dev_num` int(11) NOT NULL,
  `set_status` int(11) NOT NULL DEFAULT '0',
  `cur_status` int(11) NOT NULL DEFAULT '0',
  `reserve` timestamp NULL DEFAULT NULL,
  `repeat_start` time DEFAULT NULL,
  `repeat_stop` time DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `devices`
--

INSERT INTO `devices` (`idx`, `update_time`, `dev_num`, `set_status`, `cur_status`, `reserve`, `repeat_start`, `repeat_stop`) VALUES
(9, '2019-07-29 12:07:57', 13, 0, 0, NULL, NULL, NULL),
(10, '2019-07-29 12:07:54', 11, 0, 0, NULL, NULL, NULL),
(11, '2019-07-29 12:07:50', 10, 0, 0, NULL, NULL, NULL),
(16, '2019-07-11 05:08:55', 1, 0, 0, NULL, NULL, NULL),
(17, '2019-07-24 03:29:55', 100, 0, 0, NULL, NULL, NULL);

-- --------------------------------------------------------

--
-- Table structure for table `reserve`
--

CREATE TABLE `reserve` (
  `idx` int(11) NOT NULL,
  `dev_num` int(11) NOT NULL,
  `control` int(11) NOT NULL,
  `timestamp` timestamp NULL DEFAULT NULL,
  `time` time DEFAULT NULL,
  `day` int(11) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

--
-- Dumping data for table `reserve`
--

INSERT INTO `reserve` (`idx`, `dev_num`, `control`, `timestamp`, `time`, `day`) VALUES
(50, 8, 0, '2019-06-26 03:30:00', NULL, NULL),
(51, 8, 0, NULL, '14:00:00', 12345),
(52, 8, 1, NULL, '10:00:00', 12345),
(73, 3, 1, NULL, '08:00:00', 1234567),
(74, 3, 0, NULL, '10:00:00', 1234567),
(84, 300, 1, '2019-07-02 14:22:00', NULL, NULL),
(85, 300, 0, '2019-07-02 14:25:00', NULL, NULL),
(86, 3, 1, NULL, '18:00:00', 1234567),
(87, 3, 0, NULL, '20:00:00', 1234567),
(92, 500, 1, NULL, '08:00:00', 1234567),
(93, 500, 0, NULL, '10:00:00', 1234567),
(94, 500, 1, NULL, '18:00:00', 1234567),
(95, 500, 1, NULL, '20:00:00', 1234567),
(100, 100, 1, NULL, '08:00:00', 1234567),
(101, 100, 0, NULL, '10:00:00', 1234567),
(102, 100, 1, NULL, '18:00:00', 1234567),
(103, 100, 0, NULL, '20:00:00', 1234567);

--
-- Indexes for dumped tables
--

--
-- Indexes for table `devices`
--
ALTER TABLE `devices`
  ADD PRIMARY KEY (`idx`);

--
-- Indexes for table `reserve`
--
ALTER TABLE `reserve`
  ADD PRIMARY KEY (`idx`);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table `devices`
--
ALTER TABLE `devices`
  MODIFY `idx` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=19;

--
-- AUTO_INCREMENT for table `reserve`
--
ALTER TABLE `reserve`
  MODIFY `idx` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=106;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
