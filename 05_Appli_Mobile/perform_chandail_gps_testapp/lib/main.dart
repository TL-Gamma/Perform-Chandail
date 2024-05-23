import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:intl/intl.dart'; 

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Course Tracker',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        visualDensity: VisualDensity.adaptivePlatformDensity,
      ),
      home: CourseList(),
    );
  }
}

class CourseList extends StatefulWidget {
  @override
  _CourseListState createState() => _CourseListState();
}

class _CourseListState extends State<CourseList> {
  List<dynamic> courses = [];

  @override
  void initState() {
    super.initState();
    fetchCourses();
  }

  Future<void> fetchCourses() async {
    try {
      final response = await http.get(Uri.parse('http://192.168.4.1/gps_data'));
      if (response.statusCode == 200) {
        setState(() {
          courses = json.decode(response.body);
        });
      } else {
        throw Exception('Failed to load courses');
      }
    } catch (error) {
      // Handle error: show a Snackbar with an error message
      ScaffoldMessenger.of(context).showSnackBar(
        SnackBar(
          content: Text('Error fetching courses: ${error.toString()}'),
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: Text('Données GPS de la course'), // Dynamic title
      ),
      body: ListView.builder(
        itemCount: courses.length,
        itemBuilder: (BuildContext context, int index) {
          final course = courses[index];
          return Card(
            child: Padding(
              padding: const EdgeInsets.all(16.0),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text('Données de gps_data.json'),
                  Text('Date: ${DateFormat('yyyy-MM-dd').format(DateTime.parse(course['date']))}'), // Formatted date
                  Text('Durée: ${course['duration']}'), // Add unit label
                  Text('Vitesse moyenne: ${course['average_speed']} km/h'), // Add unit label
                  Text('Dénivlé positif: ${course['total_denivle']} m'), // Add unit label
                  Text('Distence parcourue: ${course['total_distance']} m'), // Add unit label
                ],
              ),
            ),
          );
        },
      ),
    );
  }
}
