data "archive_file" "lambda_package" {
  type        = "zip"
  source_dir  = "${path.module}/../lambda/app"
  output_path = "${path.module}/lambda.zip"
}

resource "aws_iam_role" "lambda_exec_role" {
  name = "lambda_exec_role"
  assume_role_policy = jsonencode({
    Version = "2012-10-17",
    Statement = [
      {
        Action = "sts:AssumeRole",
        Effect = "Allow",
        Principal = {
          Service = "lambda.amazonaws.com"
        }
      }
    ]
  })
}

resource "aws_iam_role_policy_attachment" "lambda_logs" {
  role       = aws_iam_role.lambda_exec_role.name
  policy_arn = "arn:aws:iam::aws:policy/service-role/AWSLambdaBasicExecutionRole"
}

resource "aws_lambda_function" "agua_lambda" {
  function_name = "agua-alert-lambda"
  filename      = data.archive_file.lambda_package.output_path
  source_code_hash = data.archive_file.lambda_package.output_base64sha256

  runtime = "python3.11"
  handler = "handler.lambda_handler"
  role    = aws_iam_role.lambda_exec_role.arn

  environment {
    variables = {
      DYNAMODB_TABLE = var.dynamodb_table_name
      SNS_TOPIC_ARN  = aws_sns_topic.alerta_agua.arn
    }
  }
}

resource "aws_lambda_permission" "allow_iot" {
  statement_id  = "AllowExecutionFromIoT"
  action        = "lambda:InvokeFunction"
  function_name = aws_lambda_function.agua_lambda.function_name
  principal     = "iot.amazonaws.com"
}
