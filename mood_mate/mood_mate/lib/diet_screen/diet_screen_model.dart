import '/flutter_flow/flutter_flow_util.dart';
import 'diet_screen_widget.dart' show DietScreenWidget;
import 'package:carousel_slider/carousel_slider.dart';
import 'package:flutter/material.dart';

class DietScreenModel extends FlutterFlowModel<DietScreenWidget> {
  ///  Local state fields for this page.

  String totCal = '100';

  String remCal = '0';

  String conCal = '0';

  ///  State fields for stateful widgets in this page.

  final unfocusNode = FocusNode();
  // State field(s) for totalCaltxt widget.
  FocusNode? totalCaltxtFocusNode;
  TextEditingController? totalCaltxtTextController;
  String? Function(BuildContext, String?)? totalCaltxtTextControllerValidator;
  // State field(s) for addCaltxt widget.
  FocusNode? addCaltxtFocusNode;
  TextEditingController? addCaltxtTextController;
  String? Function(BuildContext, String?)? addCaltxtTextControllerValidator;
  // State field(s) for Carousel widget.
  CarouselController? carouselController;

  int carouselCurrentIndex = 2;

  @override
  void initState(BuildContext context) {}

  @override
  void dispose() {
    unfocusNode.dispose();
    totalCaltxtFocusNode?.dispose();
    totalCaltxtTextController?.dispose();

    addCaltxtFocusNode?.dispose();
    addCaltxtTextController?.dispose();
  }
}
