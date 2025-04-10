data "aws_caller_identity" "current" {}

terraform {
  backend "s3" {
    bucket         = "terraform-state-agua-monitoramento-bucket"
    key            = "agua-iot/terraform.tfstate"
    region         = "sa-east-1"
    dynamodb_table = "terraform-state-lock"
    encrypt        = true
  }
}



provider "aws" {
  region = "sa-east-1"
}


resource "aws_s3_bucket" "frontend_bucket" {
  bucket = var.s3_bucket_name

  tags = {
    Name = "frontend-bucket"
  }
}

resource "aws_s3_bucket_website_configuration" "frontend_website" {
  bucket = aws_s3_bucket.frontend_bucket.id

  index_document {
    suffix = "index.html"
  }
}

# Permitir acesso público somente ao conteúdo do site
resource "aws_s3_bucket_policy" "public_policy" {
  bucket = aws_s3_bucket.frontend_bucket.id

  policy = jsonencode({
    Version = "2012-10-17",
    Statement = [
      {
        Effect = "Allow",
        Principal = "*",
        Action = "s3:GetObject",
        Resource = "${aws_s3_bucket.frontend_bucket.arn}/*"
      }
    ]
  })
}

# Bloqueios padrão desabilitados para permitir acesso público
resource "aws_s3_bucket_public_access_block" "frontend_bucket_block" {
  bucket                  = aws_s3_bucket.frontend_bucket.id
  block_public_acls       = false
  block_public_policy     = false
  ignore_public_acls      = false
  restrict_public_buckets = false
}

resource "aws_dynamodb_table" "agua_table" {
  name         = var.dynamodb_table_name
  billing_mode = "PAY_PER_REQUEST"

  hash_key  = "caixa_id"
  range_key = "timestamp"

  attribute {
    name = "caixa_id"
    type = "S"
  }

  attribute {
    name = "timestamp"
    type = "S"
  }

  tags = {
    Name = "agua-table"
  }
}



resource "aws_sns_topic" "alerta_agua" {
  name = var.sns_topic_name
}

resource "aws_sns_topic_subscription" "email_alerta" {
  topic_arn = aws_sns_topic.alerta_agua.arn
  protocol  = "email"
  endpoint  = var.alert_email
}
