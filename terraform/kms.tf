resource "aws_kms_key" "lambda_env_key" {
  description             = "KMS Key for Lambda environment variable encryption"
  deletion_window_in_days = 7
  enable_key_rotation     = true
}

resource "aws_kms_alias" "lambda_env_key_alias" {
  name          = "alias/lambda-env-key"
  target_key_id = aws_kms_key.lambda_env_key.key_id
}

resource "aws_kms_key_policy" "lambda_kms_policy" {
  key_id = aws_kms_key.lambda_env_key.id

  policy = jsonencode({
    Version = "2012-10-17",
    Id      = "lambda-env-policy",
    Statement: [
      {
        Sid: "Allow Lambda Role to use KMS for env vars",
        Effect: "Allow",
        Principal: {
          AWS: "arn:aws:iam::${data.aws_caller_identity.current.account_id}:role/lambda_exec_role"
        },
        Action: [
          "kms:Decrypt",
          "kms:GenerateDataKey"
        ],
        Resource: "*"
      },
      {
        Sid: "Allow account owner full control",
        Effect: "Allow",
        Principal: {
          AWS: "arn:aws:iam::${data.aws_caller_identity.current.account_id}:root"
        },
        Action: "kms:*",
        Resource: "*"
      }
    ]
  })
}
