import '/flutter_flow/flutter_flow_util.dart';
import 'login_singup_screen_widget.dart' show LoginSingupScreenWidget;
import 'package:flutter/material.dart';

class LoginSingupScreenModel extends FlutterFlowModel<LoginSingupScreenWidget> {
  ///  State fields for stateful widgets in this page.

  final unfocusNode = FocusNode();
  // State field(s) for TabBar widget.
  TabController? tabBarController;
  int get tabBarCurrentIndex =>
      tabBarController != null ? tabBarController!.index : 0;

  // State field(s) for emailAddress_Create widget.
  FocusNode? emailAddressCreateFocusNode;
  TextEditingController? emailAddressCreateTextController;
  String? Function(BuildContext, String?)?
      emailAddressCreateTextControllerValidator;
  // State field(s) for password_Create widget.
  FocusNode? passwordCreateFocusNode;
  TextEditingController? passwordCreateTextController;
  late bool passwordCreateVisibility;
  String? Function(BuildContext, String?)?
      passwordCreateTextControllerValidator;
  // State field(s) for password_Create2 widget.
  FocusNode? passwordCreate2FocusNode;
  TextEditingController? passwordCreate2TextController;
  late bool passwordCreate2Visibility;
  String? Function(BuildContext, String?)?
      passwordCreate2TextControllerValidator;
  // State field(s) for fnamefield widget.
  FocusNode? fnamefieldFocusNode;
  TextEditingController? fnamefieldTextController;
  String? Function(BuildContext, String?)? fnamefieldTextControllerValidator;
  // State field(s) for agefield widget.
  FocusNode? agefieldFocusNode;
  TextEditingController? agefieldTextController;
  String? Function(BuildContext, String?)? agefieldTextControllerValidator;
  bool isDataUploading = false;
  FFUploadedFile uploadedLocalFile =
      FFUploadedFile(bytes: Uint8List.fromList([]));
  String uploadedFileUrl = '';

  // State field(s) for emailAddress widget.
  FocusNode? emailAddressFocusNode;
  TextEditingController? emailAddressTextController;
  String? Function(BuildContext, String?)? emailAddressTextControllerValidator;
  // State field(s) for password widget.
  FocusNode? passwordFocusNode;
  TextEditingController? passwordTextController;
  late bool passwordVisibility;
  String? Function(BuildContext, String?)? passwordTextControllerValidator;

  @override
  void initState(BuildContext context) {
    passwordCreateVisibility = false;
    passwordCreate2Visibility = false;
    passwordVisibility = false;
  }

  @override
  void dispose() {
    unfocusNode.dispose();
    tabBarController?.dispose();
    emailAddressCreateFocusNode?.dispose();
    emailAddressCreateTextController?.dispose();

    passwordCreateFocusNode?.dispose();
    passwordCreateTextController?.dispose();

    passwordCreate2FocusNode?.dispose();
    passwordCreate2TextController?.dispose();

    fnamefieldFocusNode?.dispose();
    fnamefieldTextController?.dispose();

    agefieldFocusNode?.dispose();
    agefieldTextController?.dispose();

    emailAddressFocusNode?.dispose();
    emailAddressTextController?.dispose();

    passwordFocusNode?.dispose();
    passwordTextController?.dispose();
  }
}
