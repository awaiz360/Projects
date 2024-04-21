import '/flutter_flow/flutter_flow_util.dart';
import 'stress_relief_screen_widget.dart' show StressReliefScreenWidget;
import 'package:flutter/material.dart';

class StressReliefScreenModel
    extends FlutterFlowModel<StressReliefScreenWidget> {
  ///  State fields for stateful widgets in this page.

  final unfocusNode = FocusNode();

  @override
  void initState(BuildContext context) {}

  @override
  void dispose() {
    unfocusNode.dispose();
  }
}
